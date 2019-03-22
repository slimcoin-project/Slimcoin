#include "vanitygenwork.h"

#include <QThread>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <pthread.h>

#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/rand.h>

#include "vanity_pattern.h"
#include "vanity_util.h"

#include <string>

extern vg_context_t *vcp;

double VanityGenHashrate;
double VanityGenKeysChecked;
int VanityGenNThreads;
int VanityGenMatchCase;

QList<VanGenStruct> VanityGenWorkList;

bool VanityGenRunning;

QString VanityGenPassphrase;

bool AddressIsMine = false;


const char *version = VANITYGEN_VERSION;


/*
 * Address search thread main loop
 */

void *vg_thread_loop(void *arg)
{
    unsigned char hash_buf[128];
    unsigned char *eckey_buf;
    unsigned char hash1[32];

    int i, c, len, output_interval;
    int hash_len;

    const BN_ULONG rekey_max = 10000000;
    BN_ULONG npoints, rekey_at, nbatch;

    vg_context_t *vcp = (vg_context_t *) arg;
    EC_KEY *pkey = NULL;
    const EC_GROUP *pgroup;
    const EC_POINT *pgen;
    const int ptarraysize = 256;
    EC_POINT *ppnt[ptarraysize];
    EC_POINT *pbatchinc;

    vg_test_func_t test_func = vcp->vc_test;
    vg_exec_context_t ctx;
    vg_exec_context_t *vxcp;

    struct timeval tvstart;


    memset(&ctx, 0, sizeof(ctx));
    vxcp = &ctx;

    vg_exec_context_init(vcp, &ctx);

    pkey = vxcp->vxc_key;
    pgroup = EC_KEY_get0_group(pkey);
    pgen = EC_GROUP_get0_generator(pgroup);

    for (i = 0; i < ptarraysize; i++) {
        ppnt[i] = EC_POINT_new(pgroup);
        if (!ppnt[i]) {
            fprintf(stderr, "ERROR: out of memory?\n");
            exit(1);
        }
    }
    pbatchinc = EC_POINT_new(pgroup);
    if (!pbatchinc) {
        fprintf(stderr, "ERROR: out of memory?\n");
        exit(1);
    }

    BN_set_word(&vxcp->vxc_bntmp, ptarraysize);
    EC_POINT_mul(pgroup, pbatchinc, &vxcp->vxc_bntmp, NULL, NULL,
                 vxcp->vxc_bnctx);
    EC_POINT_make_affine(pgroup, pbatchinc, vxcp->vxc_bnctx);

    npoints = 0;
    rekey_at = 0;
    nbatch = 0;
    vxcp->vxc_key = pkey;

    vxcp->vxc_binres[0] = vcp->vc_addrtype;
    c = 0;
    output_interval = 1000;

#ifdef Q_OS_WIN32
    vanity_gettimeofday(&tvstart, NULL);
#else
    gettimeofday(&tvstart, NULL);
#endif

    if (vcp->vc_format == VCF_SCRIPT) {
        hash_buf[ 0] = 0x51;  // OP_1
        hash_buf[ 1] = 0x41;  // pubkey length
        // gap for pubkey
        hash_buf[67] = 0x51;  // OP_1
        hash_buf[68] = 0xae;  // OP_CHECKMULTISIG
        eckey_buf = hash_buf + 2;
        hash_len = 69;

    } else {
        eckey_buf = hash_buf;
        //hash_len = 65;
        hash_len = (vcp->vc_compressed)?33:65;
    }

    while (!vcp->vc_halt && VanityGenRunning) {
        if (++npoints >= rekey_at) {
            vg_exec_context_upgrade_lock(vxcp);
            /* Generate a new random private key */
            EC_KEY_generate_key(pkey);
            npoints = 0;

            /* Determine rekey interval */
            EC_GROUP_get_order(pgroup, &vxcp->vxc_bntmp,
                               vxcp->vxc_bnctx);
            BN_sub(&vxcp->vxc_bntmp2,
                   &vxcp->vxc_bntmp,
                   EC_KEY_get0_private_key(pkey));
            rekey_at = BN_get_word(&vxcp->vxc_bntmp2);
            if ((rekey_at == BN_MASK2) || (rekey_at > rekey_max))
                rekey_at = rekey_max;
            assert(rekey_at > 0);

            EC_POINT_copy(ppnt[0], EC_KEY_get0_public_key(pkey));
            vg_exec_context_downgrade_lock(vxcp);

            npoints++;
            vxcp->vxc_delta = 0;

            if (vcp->vc_pubkey_base)
                EC_POINT_add(pgroup,
                             ppnt[0],
                        ppnt[0],
                        vcp->vc_pubkey_base,
                        vxcp->vxc_bnctx);

            for (nbatch = 1;
                 (nbatch < ptarraysize) && (npoints < rekey_at);
                 nbatch++, npoints++) {
                EC_POINT_add(pgroup,
                             ppnt[nbatch],
                             ppnt[nbatch-1],
                        pgen, vxcp->vxc_bnctx);
            }

        } else {
            /*
             * Common case
             *
             * EC_POINT_add() can skip a few multiplies if
             * one or both inputs are affine (Z_is_one).
             * This is the case for every point in ppnt, as
             * well as pbatchinc.
             */
            assert(nbatch == ptarraysize);

            for (nbatch = 0;
                 (nbatch < ptarraysize) && (npoints < rekey_at);
                 nbatch++, npoints++) {

                EC_POINT_add(pgroup,
                             ppnt[nbatch],
                             ppnt[nbatch],
                             pbatchinc,
                             vxcp->vxc_bnctx);
            }
        }

        /*
         * The single most expensive operation performed in this
         * loop is modular inversion of ppnt->Z.  There is an
         * algorithm implemented in OpenSSL to do batched inversion
         * that only does one actual BN_mod_inverse(), and saves
         * a _lot_ of time.
         *
         * To take advantage of this, we batch up a few points,
         * and feed them to EC_POINTs_make_affine() below.
         */

        EC_POINTs_make_affine(pgroup, nbatch, ppnt, vxcp->vxc_bnctx);

        for (i = 0; i < nbatch; i++, vxcp->vxc_delta++) {
            /* Hash the public key */
            len = EC_POINT_point2oct(pgroup, ppnt[i],
                                     //POINT_CONVERSION_UNCOMPRESSED,
                                     (vcp->vc_compressed)?POINT_CONVERSION_COMPRESSED:POINT_CONVERSION_UNCOMPRESSED,
                                     eckey_buf,
                                     //65,
                                     (vcp->vc_compressed)?33:65,
                                     vxcp->vxc_bnctx);
            assert(len == 65 || len == 33);

            SHA256(hash_buf, hash_len, hash1);
            RIPEMD160(hash1, sizeof(hash1), &vxcp->vxc_binres[1]);

            switch (test_func(vxcp)) {
            case 1:
                npoints = 0;
                rekey_at = 0;
                i = nbatch;
                break;
            case 2:
                goto out;
            default:
                break;
            }
        }

        c += i;
        if (c >= output_interval) {
            output_interval = vg_output_timing(vcp, c, &tvstart);
            if (output_interval > 250000)
                output_interval = 250000;
            c = 0;
        }

        vg_exec_context_yield(vxcp);
    }

out:
    vg_exec_context_del(&ctx);
    vg_context_thread_exit(vcp);

    for (i = 0; i < ptarraysize; i++)
        if (ppnt[i])
            EC_POINT_free(ppnt[i]);
    if (pbatchinc)
        EC_POINT_free(pbatchinc);

    VanityGenHashrate = 0;//"0.0";

    VanityGenRunning = false;

    return NULL;
}


int VanityGenWork::start_threads(vg_context_t *vcp, int nthreads)
{
    pthread_t thread;

    while (--nthreads) {
        if (pthread_create(&thread, NULL, vg_thread_loop, vcp))
            return 0;
    }

    vg_thread_loop(vcp);
    return 1;
}

void VanityGenWork::stop_threads()
{
    if(VanityGenRunning){

        vcp->vc_halt = 1;

        vg_exec_context_t *vxcp;

        for (vxcp = vcp->vc_threads; vxcp != NULL; vxcp = vxcp->vxc_next) {
            pthread_detach((pthread_t) vxcp->vxc_pthread);//, NULL);
            vxcp->vxc_thread_active = 0;
        }
        vcp->vc_clear_all_patterns(vcp);
    }
}

int VanityGenWork::setup(){

    VanityGenRunning = true;

    EC_POINT *pubkey_base = NULL;

    vcp = vg_prefix_context_new(63, 191, 1-VanityGenMatchCase);

    vcp->vc_compressed = 1;
    vcp->vc_verbose = 0;
    vcp->vc_result_file = NULL;
    vcp->vc_remove_on_match = 1;
    vcp->vc_only_one = 0;
    vcp->vc_format = VCF_PUBKEY;
    vcp->vc_pubkeytype = 63;
    vcp->vc_pubkey_base = pubkey_base;

    vcp->vc_output_match = vg_output_match_console;
    vcp->vc_output_timing = vg_output_timing_console;

    //123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz
    //                    MNPQRSTUVWXYZabcdefghijk
    //
    //M->JKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz
    //k->123456789ABCDEFGHJKLMNPQRSTUVWXYZabcd

    int next = 0;

    for(int i = 0; i<VanityGenWorkList.length();i++){
        if(VanityGenWorkList[i].state <= 1 && VanityGenWorkList[i].privkey == "" && VanityGenWorkList[i].pattern != ""){
            next++;
        }
    }

    int npatterns = next;//VanityGenWorkList.length();

    std::string *ar = new std::string[npatterns];

    next = 0;
    for(int i = 0; i<VanityGenWorkList.length();i++){
        if(VanityGenWorkList[i].state <= 1 && VanityGenWorkList[i].privkey == "" && VanityGenWorkList[i].pattern != ""){
            ar[next] = (std::string) VanityGenWorkList[i].pattern.toLocal8Bit().constData();
            next++;
        }
    }

    const char **patterns;

    const char *vgenpat[npatterns];

    for(int i=0;i<npatterns;i++){
        vgenpat[i] = ar[i].c_str();
    }

    patterns = &vgenpat[0];

    if (!vg_context_add_patterns(vcp, (const char ** const) patterns, npatterns)){
       //To do
    }

    if (!start_threads(vcp, VanityGenNThreads))
        return 1;

}



VanityGenWork::VanityGenWork(QObject *parent) :
    QObject(parent)
{

}

void VanityGenWork::vanityGenSetup(QThread *cThread)
{
    connect(cThread, SIGNAL(started()), this, SLOT(doVanityGenWork()));
}

void VanityGenWork::doVanityGenWork()
{
    setup();//pattern,threads,caseInsensitive);
}


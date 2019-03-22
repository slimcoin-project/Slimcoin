var ecKey;
var curBlk;
var updBlk;
var curPage = 0;
var numWorkers = 0;
var workers = [];
var dcrypt_work = "";
var dcrypt_diff = "";
var mining = false;
var results = []
var lastdone = []
var dcrypt_iter = [];
var mining_interval;
var mining_start;
var mining_update_interval;
var currentHashesNeeded = 0;
var blockList = []
var stopped = true;
var best = "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";

function loadBlockInfo(blkid) {
 if (blkid == null) {  
    if ($("#blockInfoDump").html() != "") return;
    blkid = $("#be-block").html();
 }
 $.getJSON("https://web.archive.org/web/20170527180147/http://www.slimcoin.club/blockinfo/"+blkid, function(j) { 
    tbody = ""; 
    tx = j["tx"];
    for (i = 0; i < tx.length; i++) {
     maxHeight = Math.max(tx[i][1][0].length, tx[i][1][1].length)
     for (h = 0; h < maxHeight; h++) {
        tbody += "<tr valign=top>";
        if (h == 0) tbody += "<td rowspan="+maxHeight+">"+tx[i][0].substr(0,8)+"...</td>";
        tbody += "<td>";
        if (h < tx[i][1][0].length) {  
         if (tx[i][1][0][h][0] == 0) tbody += "Mined."
         else {
            tbody += tx[i][1][0][h][1][1];
            tbody += "<br />";
            tbody += (tx[i][1][0][h][0] / 1000000) + " SLM";
         }
        }
        tbody += "</td>";
        tbody += "<td>";
        if (h < tx[i][1][1].length) {
         if (tx[i][1][1][h][0] == 0) tbody += ""
         else {
            tbody += tx[i][1][1][h][1][1];
            tbody += "<br />";
            tbody += (tx[i][1][1][h][0] / 1000000) + " SLM";
         }
        }
        tbody += "</td>";
        tbody += "</tr>";
     }
    }
    delete j["tx"]; 
    $("#blockInfoDump").html( JSON.stringify(j, undefined, 2) );
    $("#txnInfoDump").html(tbody);
 });
}

function calculateBurn() {
    percent = $("#calcBurnRwd").val() / parseInt($("#be-burned").html());
    ptr = parseInt($("#be-block").html());
    while (blockList[ptr] != "" && blockList[ptr][1] != "PoB") ptr--;
    $("#calcResult").show();
    if (blockList[ptr] == "") $("#calcResult").html("No PoB found in the block history on the browser. You can browse back a bit to find one in the Block Explorer before trying.")
    else {
     SLMPerDay = parseInt(percent * 240 * blockList[ptr][2] * 1000000)/1000000 ;
     DecayOnFirstDay = (parseInt(parseFloat($("#calcBurnRwd").val()) * 1427)/1000000 );
     breakEven = parseInt(parseFloat($("#calcBurnRwd").val()) / SLMPerDay);
     WhatsLeft = parseInt(parseFloat($("#calcBurnRwd").val()) * Math.pow( (1 - 0.001427), breakEven) * 1000000) / 1000000;
     $("#calcResult").html( SLMPerDay + " SLMs generated per day, " + DecayOnFirstDay + " SLMs decayed, " + breakEven + " days to breakeven, " +  WhatsLeft + " burnt coins left at breakeven."  );
    }
}

function updatePage() {
    $("#BE-page").html( curPage + 1 );
    UpdateBlockList();
}

function prevPage() {
    if (curPage == 0) return;
    curPage -= 1;
    updatePage();
}

function nextPage() {
    curPage += 1;
    updatePage();
}

function toggleBETabs(x) {
    $(".BETabLink").removeClass("active");
    $(x).addClass("active");
    $(".BETabs").hide();
    $("#"+$(x).attr("tabName")).show();
    if ($(x).attr("tabName") == "tblBlockInfo") loadBlockInfo();

}

function startmining() {
    console.log( "Starting..." )
    $.get("http://37.187.100.75/getwork", function(data) {
        resetStatistics();
        dcrypt_work = data.split(":")[0];
        dcrypt_diff = data.split(":")[1];
        mining_start = new Date();
        mining_interval = window.setInterval( "checkwork()", 15000 )
        mining_update_interval = window.setInterval( "updateMiningStats()", 1000 );
        populateMiningInfo();
        mining = true;
        for (var i = 0; i < numWorkers; i++) {
            workers[i].postMessage( [dcrypt_work, dcrypt_diff, 4294967296 / numWorkers * i, dcrypt_iter[i], i] )
        }
    });
}

function rev256(x) {
    var rev = ""; for (i = 64; i > 0; i-=2) rev += x.substr(i-2,2); return rev;
}

function stopmining() {
    mining = false;
    window.clearInterval( mining_interval );
    window.clearInterval( mining_update_interval );
    $("#miningstat").html( "Stopped." )
    resetStatistics();
    updateMiningStats();
}

function checkwork() {
 if (mining == true) {
    $.get("http://37.187.100.75/getwork", function(data) {
        dcrypt_work = data.split(":")[0];
        dcrypt_diff = data.split(":")[1];
        populateMiningInfo();
    });
 }
}

function updateMiningStats() {
    var hashes = results.reduce( function(a,b){return a+b;} );
    var hps    = hashes * 1000 / (new Date() - mining_start);
    $("#MiningSpeed").html( parseInt(hps) )
    $("#NumWorkers").html( numWorkers )
}


/*
 "To mine a block, the block header is incremented and hashed repeatedly to try obtain a hash value of less than " + rev256(dcrypt_diff) + "<br />" +
 "This would require on average " + currentHashesNeeded / 1000000 + " million hashes before one is found. <br />" +
 "A pool does a fractional payout such that it is for example, 100 times easier to find a hash but pays out 100 times less. <br />" +
 "The pool will then, on average, score a hash every 100 attempts that will be sufficient to win the block reward. <br />" +
*/

function populateMiningInfo() {
    var info =  "<br />" + 
                 "Hashes are numbers distributed between [0, 2<sup>256</sup>) randomly. " +
                 "A small reward is awarded averaging once every 11579 hashes.<br />" +
                 "This corresponds to a number less than (in base 10): " +
                 "10000000000000000000000000000000000000000000000000000000000000000000000000<br /><br />";
    $("#miningtgt").html(info);
}

function str(x) { return x.toString() }

function AddWorkers() {
    numWorkers += 1;
    if (numWorkers == 1) startmining();
    if (workers.length < numWorkers) {
        workers.push( new Worker("slimworker.min.js") );
        results.push( 0 )
        lastdone.push( 1 )
        dcrypt_iter.push( 1000 )
        workers[workers.length - 1].onmessage = function(e) {
            e = e.data; 
            done = e[0]; stopped = e[1]; oldwork = e[2]; hit = e[3]; minerid = e[4]; miss = rev256(e[5]);
            results[minerid] += done;
            if ((new Date()).getTime() < lastdone[minerid] + 1000) dcrypt_iter[minerid] += 7;
            if ((new Date()).getTime() > lastdone[minerid] + 1000) dcrypt_iter[minerid] -= 5;
            lastdone[minerid] = (new Date()).getTime();
            console.log("Miner #"+minerid+" reporting, done "+ done+ " hashes this round, total done: " + results[minerid] + ", hit: " + miss);
            if (minerid < numWorkers) {
                if (oldwork != dcrypt_work) stopped = 4294967296 / numWorkers * minerid;
                workers[minerid].postMessage( [dcrypt_work, dcrypt_diff, stopped, dcrypt_iter[minerid], minerid] )
                if (parseInt(miss,16) < parseInt(best,16)) best = miss; 
                $("#miningprg").html( str(results.reduce( function(a,b){return a+b;} )) + " hashes so far, best so far: " + Slimcoin.ECKey.parseHexToIntString(best) );
                if (parseInt(miss, 16) < 1e73) console.log("LESS THAN: " + hit);
                if (hit != "") {
                    if (parseInt(miss, 16) < 1e73) {
                        (function() {
                            var win = miss;
                            $.getJSON("http://37.187.100.75/check/"+hit+"_"+$("#pubKey").html(), function(data) {
                                console.log( data )
                                $("#mininghis").html( "Awarded " + data["award"] + " mSLM, found after " + str(results.reduce( function(a,b){return a+b;} )) + " hashes: " + Slimcoin.ECKey.parseHexToIntString(win) + "<br />" + $("#mininghis").html() )
                                resetStatistics();
                            })
                        })();
                    }
                }
            }
        }
    }
    if (mining == true) {
        workers[numWorkers - 1].postMessage( [dcrypt_work, dcrypt_diff, stopped, dcrypt_iter[numWorkers - 1], numWorkers - 1] );
        results[numWorkers - 1] = 0;
        lastdone[numWorkers - 1] = (new Date()).getTime();
    }  
    updateMiningStats();
}

function resetStatistics() {
    for (i = 0; i < workers.length; i++) {
     results[i] = 0;
     lastdone[i] = (new Date()).getTime();
    }
    best = "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";
    mining_start = new Date();
}

function RemoveWorkers() {
    if (numWorkers == 0) return;
    numWorkers -= 1;
    if (numWorkers == 0) stopmining();
}

function EnableWallet() {
    $(".wallet_tabs").removeClass("disabled");
    $("#ww-keyinfo").show()
}

function UpdateKeyDisplay() {
     $("#prvKey").html( ecKey.toWIF() );
     $("#pubKey").html( ecKey.pub.getAddress().toString() );
     $.get("http://37.187.100.75/addr/"+$("#pubKey").html(), function(data) {
        $("#keyAmt").html( data.split(",")[0] );
        $("#keyBurnAmt").html( data.split(",")[1] );
     });
}

function genKeysFromPassPhrase() {
    ecKey = Slimcoin.ECKey.FromString( sha256_digest( $("#passphrase").val() ) );
    $.get("http://37.187.100.75/addr/"+ecKey.pub.getAddress().toString(), function(data) {
        if ((data == "0,0") || (data == "0")) {
            $(".prvKey").hide();
            ecKey = Slimcoin.ECKey.FromString( sha256_encode_hex( sha256_digest( $("#passphrase").val() ) ));
        } else {
            alert("There has been a Pubkey derivation bug found. Your funds are safe and will remain safe. However, the passphrase is incompatible with slimcoind's importpassphrase. For long term compatibility, please make a new address and transfer the funds over. \n\n You may also directly import the private key into the client.");
            $(".prvKey").show();
        }
        UpdateKeyDisplay();
        EnableWallet();
        $('#passphrase').val('');
    });
}

function genKeysFromWIF(){
    ecKey = Slimcoin.ECKey.fromWIF( $("#wifKey").val() );
    UpdateKeyDisplay()
    EnableWallet();
}

function UpdateBlockListFromCache() {
    curSt  = updBlk - curPage * 10 - 10;
    curEnd = updBlk - curPage * 10;
    var buildTbl = "";
    for (i = curEnd; i > curSt; i--) {
        buildTbl += "<tr><td onclick='loadBlockInfo($(this).html()); toggleBETabs($(\".BETabLink\")[2]);'>"+ blockList[i].join("</td><td>") +"</td></tr>";
    }
    $("#tblBlockList tbody").html(buildTbl)
}

function UpdateBlockList() {
    updBlk = parseInt( $("#be-block").html())
    if (blockList.length < updBlk) blockList.length = updBlk + 10000;
 
    curSt  = updBlk - curPage * 10 - 10;
    curEnd = updBlk - curPage * 10;
    for (; curSt <= curEnd; curSt++) {
        if (blockList[curSt] == null) break;
    }
    for (; curSt <= curEnd; curEnd--) {
        if (blockList[curEnd] == null) break;
    }
    if (curEnd < curSt) UpdateBlockListFromCache();
    else {
        $.get("http://37.187.100.75/block/"+(curSt-1).toString()+"-"+curEnd, function(data) {
        var blks = JSON.parse( data )
        for (i in blks) {
            blockList[ blks[i][0] ] = blks[i];
        }
        UpdateBlockListFromCache();
        });
    }
}

function UpdateBlockExplorer() {
 $.get("http://37.187.100.75/getinfo?"+$("#be-block").html(), function(data, status) {
    if (status == "notmodified") return;
    info = data.split("\n");
    $("#be-block").html(info[2]);
    $("#be-diff").html(info[4]);
    $("#be-hps").html( parseInt(info[5]/1000)/1000 );
    $("#be-burned").html( parseInt(info[7] / 1000000));
    $("#be-total").html( parseInt(info[1]) );
    currentHashesNeeded = parseInt(info[6]);
    if (info[2] != curBlk) UpdateBlockList();
 });
}

function estFee(tx) {
    var baseFee = Slimcoin.networks.slimcoin.feePerKb;
    var byteSize = tx.toBuffer().length

    var fee = baseFee * Math.ceil(byteSize / 1000)
    //for (var o in tx.outs)
    // if (tx.outs[0].script.getHash().toString("hex") == "eb61aeac80f8ecb218045f5b96988948c6654d35") fee = 0; // Burns are fee-free - supposed to be :(
    return fee;
} 

$(document)
    .ready(function() {
        UpdateBlockExplorer();
        for (i = 0; i < $("a").length; i++) if ($("a")[i].href.indexOf("#") > -1) $("a")[i].target="_parent";

        // check for GET strings
        window.setTimeout( function() {
             $('html, body').animate({
                scrollTop: $("a[name='"+get[0].split("#")[1]+"']").offset().top
             }, 500);
            }, 1000);
        
        var get = document.URL.split("?");
        if (get.length == 2) {
            cmd = get[1].split("&")[0];
            if (cmd.split("=")[0] == "block") {
             loadBlockInfo(cmd.split("=")[1]); toggleBETabs($(".BETabLink")[2]);
            }
        }

        window.setInterval("UpdateBlockExplorer()", 60000);  
        $(".wallet_tabs").click( function() {
         if ($(this).hasClass("disabled")) return;
         $(".wallet_tabs").removeClass("active")
         $(this).addClass("active");
         $(".wallet_tabbox").hide()
         $("#"+$(this).attr("assocBox")).show();
        });

        $.get("/addr/STtv9aBsVi1R1q72xmbwbHJHCUTX8KcAhP", function(data) {
         //Sg72f5icXXAjrdV7o15ZrFdj9CvNaTZwS1 is an old donation address, key held in cold storage, lazy to combine
         $("#donate_bounty_value"  ).html( (data.split(",")[0]/1000000+15999.98) + " SLM");
        });

        $.get("/addr/SgDk5KZjRGKZavC2Dk4cd4Cpss1cGoyUG6", function(data) {
         $("#donate_giveaway_value").html(data.split(",")[0]/1000000 + " SLM");
        });

        var txn;

        $("#sendslmbutton").click( function () {
         if (parseInt($("#sendAmt").val()) > parseInt($("#keyAmt").html())) {
            alert("Insufficient value");
            return;
         }
         this.value="Creating transaction...";
         $.get("http://37.187.100.75/txn/" + $("#pubKey").html(), function(data) { 
            txsum  = 0;
            txnum  = 0;
            txlist = JSON.parse( data )
            tx     = new Slimcoin.Transaction();
            prvKey = Slimcoin.ECKey.fromWIF( $("#prvKey").html() )

            tx.addOutput( $("#sendTo").val(), parseInt($("#sendAmt").val()) );

            for (var inhash in txlist) {
             for (var inhidx in txlist[inhash]) {
                txhidx_val = txlist[inhash][inhidx];
                if (txhidx_val > 0) {
                 tx.addInput( inhash, parseInt(inhidx) );
                 txsum += txhidx_val;
                 txnum += 1;
                }
                //if (txsum >= parseInt($("#sendAmt").val()) + estFee(tx) ) break;
             }
             //if (txsum >= parseInt($("#sendAmt").val()) + estFee(tx) ) break;
            }
            for (var i = 0; i < txnum; i++)
             tx.sign( i, prvKey ); 
            if ((txsum <= parseInt($("#keyAmt").html()) ) && (txsum >= parseInt($("#sendAmt").val()) + estFee(tx) )) {
             if (txsum - parseInt($("#sendAmt").val()) - estFee(tx) > 0)
                tx.addOutput( $("#pubKey").html(), txsum - parseInt($("#sendAmt").val()) - estFee(tx) );
             for (var i = 0; i < txnum; i++)
                tx.sign( i, prvKey ); 
             console.log( tx.toHex() );
             $.get("http://37.187.100.75/bcast/" + tx.toHex(), function(data) {
                console.log("Transaction broadcasted: " + data);
                if ("message" in data)
                 alert("Transaction broadcasted, reply from node: " + data["message"]); 
                else
                 alert("Transaction successful, TX ID: " + data); 
             });
            } else {
             alert("Insufficient value after fees. Fee required:" + estFee(tx) );
            }
         });
        });
        
        var
            changeSides = function() {
                $('.ui.shape')
                    .eq(0)
                        .shape('flip over')
                        .end()
                    .eq(1)
                        .shape('flip over')
                        .end()
                    .eq(2)
                        .shape('flip back')
                        .end()
                    .eq(3)
                        .shape('flip back')
                        .end()
                ;
            },
            validationRules = {
                firstName: {
                    identifier  : 'email',
                    rules: [
                        {
                            type   : 'empty',
                            prompt : 'Please enter an e-mail'
                        },
                        {
                            type   : 'email',
                            prompt : 'Please enter a valid e-mail'
                        }
                    ]
                }
            }
        ;

        $('.ui.dropdown')
            .dropdown({
                on: 'hover'
            })
        ;

        $('.ui.form')
            .form(validationRules, {
                on: 'blur'
            })
        ;

        $('.masthead .information')
            .transition('scale in')
        ;

    })
;

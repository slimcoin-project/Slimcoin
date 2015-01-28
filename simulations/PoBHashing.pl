use strict;
use warnings;

#simple SHA256 hashing
use Digest::SHA qw(sha256_hex);
use Digest::MD5 qw(md5_hex);
use bignum;

#sub hash($) { return sha256_hex($_[0]); }
sub hash($) { return md5_hex($_[0]) . md5_hex($_[0] . "HI"); }

sub avg_array 
{
    my $total = 0;
    foreach my $ele (@_)
    {
        $total += $ele;
    }

    return $total / ~~@_;
}

sub bn_to_hex
{
    my $hex = $_[0];
    my $nhex_chars = length($hex->as_hex()) - 2; #-2 for the 0x in front

    #print $hex, " ", $nhex_chars, "\n";

    #get the 0x000.. into a string
    my $out_hex = "0x" . "0" x (64 - $nhex_chars);

    #concatonate the rest of the hex to it
    $out_hex .= substr($hex->as_hex, 2, $nhex_chars);

    return $out_hex;
}

my $target = hex("0x0300f280d422f4418366931cebcfbd17f5ede1507a951153b0f15a21c10de0");

sub main
{
    my ($nNonce, $scale, $nminers) = (0, 50, 200);
    my ($i, $b) = (0, 0);
    my @average;
    my $hash_out = hex("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

    #printf("Target %s\n\n hashout %s\n", bn_to_hex($target), $hash_out);

    #scale the target
    $target *= $scale;

    for(; $b < $nminers; $b++)
    {
        #average the 50 nonces per miner
        for($i = 0; $i < 50; $i++)
        {
            for($nNonce = 0; !$nNonce || $hash_out >= $target; $nNonce++)
            {
                $hash_out = hex(hash("$nminers $i $nNonce"));
            }

            #append the $nNonce to the array
            push(@average, $nNonce);
        }

        printf("Miner %d DONE\n", $b);
    }

    #divide the average by the number of miners because they are all mining at the same time
    printf("Average %f using scale %f with %d miners\n", avg_array(@average) / $nminers, $scale, $nminers);

    return;
}

main();

#use bignum;
use Math::BigFloat;
use constant COIN => 1000000;
use constant BURN_CONSTANT => 0.1 * COIN / 100;
use constant BURN_HARDER_TARGET => 0.5;

sub main()
{
    my $rate = 1.000693388;
    my ($start1, $start2) = (1000 * COIN, 1000 * COIN);
    my $i;
    for($i = 0; $i < 1000; $i++)
    {
        printf("Values are %f %f, difference %f\n", $start1, $start2, $start2 - $start1);
        $start1 = int($start1 / $rate);
        $start2 /= $rate;
    }

    return;
}

main();

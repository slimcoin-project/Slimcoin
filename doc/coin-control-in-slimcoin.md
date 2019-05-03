# Coin control in Slimcoin

## What is coin control?

When you send Slimcoins to someone else, the client makes a pseudo-random which address will used as the source of the coins. With coin control you can pecisely choose which address(es) will be the sending address(es) and which of the unspent outputs will be the sending inputs.
  
## What is an unspent output?

Lets say your Slimcoin address is 111. Suppose someone sends 1 SLM to this address. Now address 111 has one unspent output. Now suppose someone else sends 2 SLM to this address. Address 111 now has two unspent outputs and a balance of 3 BTC in total. Let’s say you want to send someone 0.1 BTC.

Ideally, one could say "Okay, simply subtract 0.1 BTC from the first unspent output and send it". But this is not possible because the bitcoin protocol (upon which SLimcoin is based) works differently. You always have to spent the whole output. This means in this case the Slimcoin client would take the first unspent output, send 0.1 BTC to the other person and 0.9 BTC back to yourself. For this, the client creates a new change address in the background and adds this address to your wallet. This “change address” would then have one unspent output of 0.9 BTC. This means every wallet has kinda “hidden” change addresses. For example if you now would lose your wallet but still know the private key of 111, you would be only able to get the 2 BTC back. The 0.9 BTC from the change address would be inaccessible because you would have to know the private key of the change address.
  
## Why coin control?

The first version of coin control was called "Patching The Bitcoin Client To Make It More Anonymous". Sometimes when you receive Slimcoins, the sender or even the public knows that it is you who owns the Slimcoin address. Now if you later want to make a truly anonymous payment, you shouldn’t use those received coins. Also you get full control over your unspent outputs and can do things like choosing which address actually owns the coins, clean them up or send all change back to the origin address etc. You can also prevent the Slimcoin client from creating a change address by sending the exact amount of the selected unspent outputs minus the fee. Also you get a better impression of what’s going on in your wallet and get a better understanding of the bitcoin protocol.

## Changes to the GUI

Screenshots
- ![Coincontrol preferences](https://github.com/gjhiggins/Slimcoin/raw/master/doc/coincontrol-01.png)
- ![Send coins dialog enable coin control panel](https://github.com/gjhiggins/Slimcoin/raw/master/doc/coincontrol-02.png)
- ![Coin control tree mode](https://github.com/gjhiggins/Slimcoin/raw/master/doc/coincontrol-03.png)
- ![Coin control list mode](https://github.com/gjhiggins/Slimcoin/raw/master/doc/coincontrol-04.png)
- ![Send coins dialog, fully populated](https://github.com/gjhiggins/Slimcoin/raw/master/doc/coincontrol-05.png)

## Main

Settings checkbox "Display coin control features (experts only!)" (default=no)

## Tab Send coins

Button Inputs
click on this button opens actual coin control dialog. If no Inputs are selected "automatically selected" is shown.
Change Checkbox
checked -> provide custom change address

## Coin Control Dialog

Shows a list of all unspent outputs with two view modes:

-  tree mode: outputs including change are grouped by wallet address
     tree can be opened showing the actual outputs for this wallet address including change if change, the change address is shown in column "address", otherwise the column "address" is empty because it's a direct output of the wallet address having the address already shown in the parent node (same with label)
- list mode: simple list of all unspent outputs

Select outputs by checkbox - only the checked outputs are used when sending a transaction. If none are selected then coin control is inactive (just as normal). Check/uncheck all by clicking on "(Un)select all"

Sort colums: the tooltip available in column list mode in the column label for the change shows from which address the change came from.

Context menu
- Copy to clipboard (amount,label,address,transaction id,lock,unlock)
Labels at the top
- Quantity: number of selected outputs
- Amount: sum of selected unspent outputs
- Fee:   see "Calculation of fee and transaction size" below minus fee: simply the amount shown is "Selected" minus the amount shown in "Fee"
- Bytes: see "Calculation of fee and transaction size" below
- Priority: priority = coinage / transactionsize. coinage = value * confirmations.  miners order transactions by priority when selecting which go into a block
- Low Output: "yes" if any recipient receives an amount < 0.01BTC
- Change: shows the change you get back

Right click the labels for copy amount to clipboard
   
## Selection

In this version of coin control, all selected outputs are going into the transaction for sure!!
Of course, if you select more than you actually send, the bitcoin core will send the rest back to you as change, just as normal.
And of course, if you select less than you send you will get "The amount exceeds your balance".
And as already mentioned, if none are selected, coin control is inactive, this means everything is just the same as without coin control.
  
## Fee

If the sum of selected outputs minus the amount you are going to send is smaller than the required fee, you will probably get
"The total exceeds your balance when the transaction fee is included"
This is because you didnt select enough outputs to pay the fee.
You always must select enough outputs, so that those outputs can pay the fee.
  
## Calculation of fee and transaction size

The fee is calculated according to the fee set in the Settings menu.
The calculation assumes 2 outputs in total. One for the destination address and one for the change.
The formula is nBytesOutputs + (2 * 34) + 10. nBytesOutputs is the sum of selected outputs, 148 or 180 bytes per output, depending if compressed public key.
Due to the inner workings of bitcoin the size per output is actually +/- 1 byte. Meaning the shown calculation is not always 100% correct. 
If you send exactly "selected minus fee" then you will not have change (1 output only). The transaction will then be 34 bytes smaller as what was calculated before.

## Free Transactions

In order to be able to send a free transaction, you need to follow the rules:

     - transaction size must be < 10000 bytes
     - priority must be at least "medium"
     - any recipient must receive at least 0.01BTC
     - change must be either zero or at least 0.01BTC

If you violate one rule you will see a min-fee and also the labels turn red: Bytes.Priority, Low Output, Change. Depending which rule you violated.  Those 4 labels also have tool tips explaining this.  Also remember that violating one of the first 2 rules means 0.0005 PER kilobyte min-fee, while violating one of the last 2 means 0.0005 min-fee only.

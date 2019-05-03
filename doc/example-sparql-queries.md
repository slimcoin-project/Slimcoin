## SPARQL queries for interrogating the RDF graph expression of the blockchain

Maximum block height:

    PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>

    SELECT ?subject ?height
    WHERE {
      ?subject rdf:type <http://purl.org/net/bel-epa/ccy#Block> .
      ?subject <http://purl.org/net/bel-epa/ccy#height> ?height
    }
    ORDER BY DESC(?height)
    LIMIT 1


Get input and output tx for an address:

    PREFIX ccy: <http://purl.org/net/bel-epa/ccy#>
    SELECT DISTINCT ?txo ?datetime ?value ?stype WHERE {
     ?txo ccy:address ccy:VFouUCS3gM3yrTdKHkyKoLGbPcWC8mp4A4 .
     ?tx ccy:output ?txo . ?tx ccy:time ?datetime .
     ?txo ccy:value ?value . ?txo ccy:type ?stype
    } ORDER BY DESC(?datetime)


List all inscriptions:

    PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>

    SELECT DISTINCT(?inscription as ?inscriptions)
    WHERE {
      ?subject rdf:type <http://purl.org/net/bel-epa/ccy#TransactionOutput> .
      ?subject <http://purl.org/net/bel-epa/ccy#inscription> ?inscription
    }

## RDF representation

A block:

    :C0000000733567067927d0778bdee1ad3034ecc60e49cfc3ae05dbc3a1eddbb60 a :Block ;
        :difficulty 0.06464541 ;
        :flags "proof-of-work"^^xsd:string ;
        :height 759 ;
        :mint 6.19 ;
        :nextblockhash "None"^^xsd:hexBinary ;
        :previousblockhash :C8fe77bb34c5aec9206a3c73beccaf1fc53e6d4afe297c3782f57c09e4eb9c627 ;
        :size 355 ;
        :time 1492346152 .

A Transaction:

    :C002bb7d860336cb06e282dba0d84f9247300e24b2e2a8880c127323ab5521e6b a :Transaction ;
        :blockhash <C0000002cb1b737ffd109639e3f7e1f81959dffd602e58d1ad6be9720215c5f7b> ;
        :input :C002bb7d860336cb06e282dba0d84f9247300e24b2e2a8880c127323ab5521e6b-I-0 ;
        :output :C002bb7d860336cb06e282dba0d84f9247300e24b2e2a8880c127323ab5521e6b-O-0 ;
        :time 1492284846 .

A TransactionInput:

    :C002bb7d860336cb06e282dba0d84f9247300e24b2e2a8880c127323ab5521e6b-I-0 a :TransactionInput ;
        :coinbase "04a675f2580101062f503253482f"^^xsd:hexBinary .

A TransactionOutput:

    :C002bb7d860336cb06e282dba0d84f9247300e24b2e2a8880c127323ab5521e6b-O-0 a :TransactionOutput ;
        :address :mgeUFf7a8dC94yJ5BMbtVKWur1jTrsnR4C ;
        :n 0 ;
        :pkasm "02f07a96082739cb88c5207969d0786771b7a2bcaf3c0fb0c470375d62850a0a5a OP_CHECKSIG"^^xsd:string ;
        :type "pubkey"^^xsd:string ;
        :value 8.34 .

## SPARQL queries

Identify the TransactionOutput for a given inscription:

    SELECT ?subject
    WHERE {
      ?subject rdf:type <http://purl.org/net/bel-epa/ccy#TransactionOutput> .
      ?subject <http://purl.org/net/bel-epa/ccy#inscription> "Hello world."
    }


results:

    <http://purl.org/net/bel-epa/ccy#C3ad3a26c024a0094f88a1b2de4c7f3129575b1fd6c89d3648e4c68f5b8cf4b25-O-2>

Identify the Transaction containing the TransactionOutput:

    SELECT ?tx
    WHERE {
      ?tx rdf:type <http://purl.org/net/bel-epa/ccy#Transaction> .
      ?tx <http://purl.org/net/bel-epa/ccy#output> <http://purl.org/net/bel-epa/ccy#C3ad3a26c024a0094f88a1b2de4c7f3129575b1fd6c89d3648e4c68f5b8cf4b25-O-2> .
    }

results:

    <http://purl.org/net/bel-epa/ccy#C3ad3a26c024a0094f88a1b2de4c7f3129575b1fd6c89d3648e4c68f5b8cf4b25>

Identify the Block containing the Transaction:

    SELECT ?b
    WHERE {
      <http://purl.org/net/bel-epa/ccy#C3ad3a26c024a0094f88a1b2de4c7f3129575b1fd6c89d3648e4c68f5b8cf4b25> <http://purl.org/net/bel-epa/ccy#blockhash> ?b .
    }

results:

    <http://purl.org/net/bel-epa/ccy#C000000061900945721ae028b98844c8717bed57ab3f10d8720c8f96f9d530e34>


Show all statements about the Block:

    SELECT ?pred ?obj
    WHERE {
      <http://purl.org/net/bel-epa/ccy#C000000061900945721ae028b98844c8717bed57ab3f10d8720c8f96f9d530e34> ?pred ?obj .
    }

results:

    1.  rdf:type                                          <http://purl.org/net/bel-epa/ccy#Block>
    2.  <http://purl.org/net/bel-epa/ccy#difficulty>      0.06490015
    3   <http://purl.org/net/bel-epa/ccy#flags>           "proof-of-work"
    4   <http://purl.org/net/bel-epa/ccy#height>          138
    5   <http://purl.org/net/bel-epa/ccy#mint>            6.19
    6   <http://purl.org/net/bel-epa/ccy#nextblockhash>   <http://purl.org/net/bel-epa/ccy#C7a52155b2a89ae0ce461742013ec43d55b66c11410c4f5094cfeb747622f235a>
    7   <http://purl.org/net/bel-epa/ccy#previousblockhash> <http://purl.org/net/bel-epa/ccy#C3bd837d3371108666e1c9c5967d68f7bb95df597b4d0de9dd754f975d1df32c6>
    8   <http://purl.org/net/bel-epa/ccy#size>            588
    9   <http://purl.org/net/bel-epa/ccy#time>            1492283025


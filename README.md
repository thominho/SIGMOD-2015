# SIGMOD-2015
SIGMOD-2015
https://db.in.tum.de/sigmod15contest/task.html

It is written in C++. A simple makefile to compile all the source files is included. To run write ./main followed by arguments
-enable_validation_hashtable=argv[1] -thread_numbers=argv[2].

-Journal Array for every query that is going to be inserted.
-Extendable Hashtable for cleaning queries collisions. Faster searching and/or insertion.
-Zombie List for cleaning queries that propably will not be searched/deleted again.
-Threads for faster execution of the database system.

TO DO
-Better documentation
-Cleaning Code
-Better makefile and script to execute the program

MUST
-OPTIMIZE IT A LOT.Hashtable insert/delete registries in a very bad mannor. Firstly we collect both insertions and deletions.Afterwards it testing for duplicates of both insertions and deletions. This could (probably) be improved inserting or deleting while parsing query. In addition, optimizing iterators (i.e. for loops).


Task Details

The ACID principle, mandating atomicity, consistency, isolation and durability is one of the cornerstones of relational database management systems. In order to achieve the third principle, isolation, concurrent reads and writes must be handled carefully.

In practice, transaction processing is often done in an optimistic fashion, where queries and statements are processed first, and only at the commit point the system validates if there was a conflict between readers and writers.

In this challenge, we simulate this in a slightly simplified way: The system processes already executed transactions and only needs to check efficiently whether concurrent queries conflict with them. So, in short: we issue a list of insert and delete statements and your program needs to figure out whether given predicates match the inserted or deleted data.

Each client has to process a number of requests, provided via standard input, and must provide answers via stdout. The protocol itself is a simple binary protocol, which is described in the C++ reference implementation. It also contains the testdriver and a basic test case to verify the functionality of your program locally. Additionally there are also a small and a medium-sized test case which can be used together with the test driver; the latter can be decompressed using tar xpvf data_medium.tar.xz. We also provide unoffical untested stub implementations in Go, Java and Rust.

A basic description of the workload is described in the following section. After the examples there is a detailed description of the semantics of each request type.

During the contest we will also hand out larger test datasets, similar to those used during evaluation.

Example Session

    In the following we give a short example of the challenge task. Note that for this example we use a human-readable variant of our protocol; the actual implementation protocol is defined in the next section.

    Each session starts by first defining a schema (in this case 2 relations with 3 and 4 columns each), and loading some initial data (4 tuples in relation 0, 3 in relation 1) in transaction 0. The values in the first column are always unique in each relation (primary key constraint).

        defineschema [3 4]
        transaction 0 [] [
           0 [1 1 2     2 1 2     3 4 5     7 7 7]
           1 [1 0 0 0    3 0 0 1     4 1 1 1]
        ]

    Further, consider the following three update transactions. Transaction 1 inserts the tuple [6 5 4] into relation 0, transaction 2 deletes a tuple identified by the first column value 4 from relation 1, and transaction 3 both deletes and inserts a tuple.

       transaction 1 [] [0 [6 5 4]]
       transaction 2 [1 [4]]
       transaction 3 [0 [3]] [0 [3 5 6]]

    Now, consider 3 validation requests that have to be validated against given transaction ranges. Validations 0 and 1 check transactions 1-2, while validation 2 checks all transactions 1-3.

        validation 0 1 2 [0 c0=4] [1 c1>8]
        validation 1 1 2 [1 c2=1]
        validation 2 1 3 [0 c0=3 c1=2] [0 c2=4]

    The predicate in validation 1 could be written as (exists t in r1 | t.c2=1), which checks whether there is a tuple t in relation 1 for which the column value t.c2 equals 1. Similarly, the predicte in validation 2 could be written as (exists t in r0 | t.c0=3 and t.c1=2) or (exists t in r0 | t.c2=4).
    Now, if we check whether these predicates are affected by the indicated transactions
        validation request 0 is not conflicting, because no tuples were modified where c0=4 in R0 or where c1>8 in R1),
        validation request 1 is conflicting (with transaction 2), as the deleted tuple had the value c2=1, and
        validation request 2 is conflicting (with transaction 1). Note that the "[0 c0=3 c1=2]" part is not conflicting with transaction 3, as the c1=2 condition was not met, but "[0 c2=4]" conflicted with the insert.
    The expected output of the program is therefore
        0
        1
        1
    Indicating the validation requests with conflicts (the second and third one).

Workload Specification
Each client has to process a number of requests, provided via standard input, and must provide answers via stdout. The protocol itself is a simple binary protocol, which is described in the reference implementation. Here, we discuss the semantics of the six message types.

    Define Schema
    Format   	defineschema [<columnCounts>, ...]
    This will always be the first message in each data set, and defines the number of relations and the number of columns within each relation. Both relation numbers and column numbers are zero based. Columns are always unsigned 64-bit integers. The first column of every relation is the primary key. Note that relations can differ greatly in size, and that value distributions can differ greatly between columns. An implementation can assume that there will not be more than 10,000 relations and not more than 1,000 columns per relation.
    Transaction
    Format   	transaction <transactionId> [<deleteOperations>, ...] [<insertOperations>, ...]

    A transaction consists of a number of delete operations followed by a number of insert operations. Transaction Ids are monotonic increasing, and transactions are executed in id order. Deletions modeled as

    delete <relationId> [<keys>, ...]

    identifying each tuple by its primary key (i.e., the first column). It can happen that a row is already deleted. Insertions are modeled as

    insert <relationId> [<values>, ...]

    giving the new tuples as streams of values. Inserts will not cause primary key violations (the same transaction might have deleted the tuple before, though).
    Validation
    Format   	validation <validationId> <fromTransaction> <toTransaction> [<queries>, ...]

    A validation request consists of a list of conjunctive queries (see below). The validation fails (i.e., the request is conflicting) if any of the conjunctive queries is satisfied by any tuple that is inserted or deleted by a transaction from the transaction range (both from and to are inclusive). Validation ids will be dense and monotonic increasing.

    The conjunctive queries are given as

    <relationId> [(<column> <operation> <constant>), ...]

    where operation is in {=,!=,<,<=,>,>=}.

    Note that the distribution of operations is non-uniform. Tests for equality are much more common than other operations, and some columns will be tested more frequent than others.
    Flush
    Format   	flush <validationId>

    All messages up to now did not produce client output, the client was free to re-arrange them and execute them as it seemed fit. The flush request triggers the output of all queries up to this point (including), in validationId order, forcing the client to produce the character '0' if the validation succeeded (i.e., there is no conflict), and a '1' if there was a conflict. As some progamming languages buffer stdout and only flush after a newline, you might need to manually flush the output so that it is send to the driver.
    Forget
    Format   	forget <transactionId>

    At some point old transactions are no longer relevant for new incoming transactions. The forget request allows to free all memory up to the given transactionId (including). Future validation requests are guaranteed to not ask for transaction ranges that include "forgotten" transactions.
    Done

    This message is always sent as last message of the data set to terminate the program. It is mainly useful for debugging to see if the input stream was parsed correctly.


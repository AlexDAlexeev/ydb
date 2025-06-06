# Scan Queries in {{ ydb-short-name }}

Scan Queries is a separate data access interface designed primarily for running analytical ad hoc queries against a DB.

This method of executing queries has the following unique features:

* Only *Read-Only* queries.
* In `SERIALIZABLE_RW` mode, a data snapshot is taken and then used for all subsequent operations. As a result, the impact on OLTP transactions is minimal (only taking a snapshot).
* The output of a query is a data stream ([gRPC stream](https://grpc.io/docs/what-is-grpc/core-concepts/)). This means scan queries have no limit on the number of rows in the result.
* Due to the high overhead, it is only suitable for ad hoc queries.

{% note info %}

From the *Scan Queries* interface, you can query [system tables](../../dev/system-views.md).

{% endnote %}

Scan queries cannot currently be considered an effective solution for running OLAP queries due to their technical limitations (which will be removed in time):

* The query duration is limited to 5 minutes.
* Many operations (including sorting) are performed entirely in memory, which may lead to resource shortage errors when running complex queries.
* A single strategy is currently in use for joins: *MapJoin* (a.k.a. *Broadcast Join*) where the "right" table is converted to a map; and therefore, must be no more than a few gigabytes in size.
* Prepared form isn't supported, so for each call, a query is compiled.
* There is no optimization for point reads or reading small ranges of data.
* The SDK doesn't support automatic retry.

For handling OLAP workloads in {{ ydb-short-name }}, there is a specialized type of table — [column-oriented](../datamodel/table.md#column-oriented-tables) tables. These tables store the data of each column separately from other columns. This allows only the columns directly involved in the query to be read during execution.
{% note info %}

Despite the fact that *Scan Queries* obviously don't interfere with the execution of OLTP transactions, they still use common DB resources: CPU, memory, disk, and network. Therefore, running complex queries **may lead to resource hunger**, which will affect the performance of the entire DB.

{% endnote %}

## How Do I Use It? {#how-use}

Like other types of queries, *Scan Queries* are available via the {% if link-console-main %}[management console]({{ link-console-main }}) (the query must specify `PRAGMA Kikimr.ScanQuery = "true";`),{% endif %} [CLI](../../reference/ydb-cli/commands/scan-query.md), and [SDK](../../reference/ydb-sdk/index.md).

{% if oss %}

### C++ SDK {#cpp}

To run a query using *Scan Queries*, use two methods from the `Ydb::TTableClient` class:

```cpp
class TTableClient {
    ...
    TAsyncScanQueryPartIterator StreamExecuteScanQuery(const TString& query,
        const TStreamExecScanQuerySettings& settings = TStreamExecScanQuerySettings());

    TAsyncScanQueryPartIterator StreamExecuteScanQuery(const TString& query, const TParams& params,
        const TStreamExecScanQuerySettings& settings = TStreamExecScanQuerySettings());
    ...
};
```

{% endif %}


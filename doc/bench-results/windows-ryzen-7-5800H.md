```
> date
The current date is: Wed 28 Oct 2026

> ver
Microsoft Windows [Version 10.0.26200.9711]

> powershell "(Get-CimInstance -ClassName Win32_Processor).Name"
AMD Ryzen 7 5800H with Radeon Graphics

❯ cl
Microsoft (R) C/C++ Optimizing Compiler Version 19.51.36260 for x64
```

## Json Parse

`> bin\bench-pojobuf-json-parse.exe --samples=30`

## canada.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     3.084 | 3084100 |      - |      324.2
 pojobuf-mut charconv     |       1 |    14.341 |14340800 |  4.650 |       69.7
 sajson-mut               |       1 |     3.613 | 3613300 |  1.172 |      276.8
 pojobuf-const            |       1 |     3.108 | 3108500 |  1.008 |      321.7
 sajson-const             |       1 |     3.765 | 3764800 |  1.221 |      265.6
 simdjson                 |       1 |     2.822 | 2822000 |  0.915 |      354.4
 boost                    |       1 |     3.808 | 3808100 |  1.235 |      262.6
 nlohmann                 |       1 |    38.895 |38894700 | 12.611 |       25.7

## citm_catalog.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     1.201 | 1200600 |      - |      832.9
 pojobuf-mut charconv     |       1 |     1.228 | 1228000 |  1.023 |      814.3
 sajson-mut               |       1 |     1.259 | 1259300 |  1.049 |      794.1
 pojobuf-const            |       1 |     1.299 | 1299300 |  1.082 |      769.6
 sajson-const             |       1 |     1.514 | 1513600 |  1.261 |      660.7
 simdjson                 |       1 |     0.875 |  875300 |  0.729 |     1142.5
 boost                    |       1 |     1.478 | 1478200 |  1.231 |      676.5
 nlohmann                 |       1 |    10.447 |10446800 |  8.701 |       95.7

## gsoc-2018.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     2.848 | 2848100 |      - |      351.1
 pojobuf-mut charconv     |       1 |     2.843 | 2842600 |  0.998 |      351.8
 sajson-mut               |       1 |     2.481 | 2481300 |  0.871 |      403.0
 pojobuf-const            |       1 |     3.370 | 3369700 |  1.183 |      296.8
 sajson-const             |       1 |     2.947 | 2947300 |  1.035 |      339.3
 simdjson                 |       1 |     1.357 | 1356500 |  0.476 |      737.2
 boost                    |       1 |     1.983 | 1983300 |  0.696 |      504.2
 nlohmann                 |       1 |    17.906 |17906000 |  6.287 |       55.8

## marine_ik.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     4.190 | 4190300 |      - |      238.6
 pojobuf-mut charconv     |       1 |    10.687 |10687100 |  2.550 |       93.6
 sajson-mut               |       1 |     5.172 | 5171900 |  1.234 |      193.4
 pojobuf-const            |       1 |     4.251 | 4250800 |  1.014 |      235.2
 sajson-const             |       1 |     5.491 | 5490700 |  1.310 |      182.1
 simdjson                 |       1 |     4.101 | 4101500 |  0.979 |      243.8
 boost                    |       1 |     6.349 | 6348800 |  1.515 |      157.5
 nlohmann                 |       1 |    42.352 |42352100 | 10.107 |       23.6

## mesh.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     1.145 | 1145000 |      - |      873.4
 pojobuf-mut charconv     |       1 |     3.746 | 3745600 |  3.271 |      267.0
 sajson-mut               |       1 |     1.387 | 1387100 |  1.211 |      720.9
 pojobuf-const            |       1 |     1.169 | 1169000 |  1.021 |      855.4
 sajson-const             |       1 |     1.441 | 1440800 |  1.258 |      694.1
 simdjson                 |       1 |     1.051 | 1051100 |  0.918 |      951.4
 boost                    |       1 |     1.634 | 1633800 |  1.427 |      612.1
 nlohmann                 |       1 |    10.766 |10765700 |  9.402 |       92.9

## mesh.pretty.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     1.635 | 1634800 |      - |      611.7
 pojobuf-mut charconv     |       1 |     4.514 | 4514000 |  2.761 |      221.5
 sajson-mut               |       1 |     1.765 | 1764600 |  1.079 |      566.7
 pojobuf-const            |       1 |     1.677 | 1676900 |  1.026 |      596.3
 sajson-const             |       1 |     2.023 | 2022900 |  1.237 |      494.3
 simdjson                 |       1 |     1.367 | 1367000 |  0.836 |      731.5
 boost                    |       1 |     1.703 | 1702900 |  1.042 |      587.2
 nlohmann                 |       1 |    13.482 |13481700 |  8.247 |       74.2


## POJO Travese

`> bin\bench-pojobuf-traverse.exe --samples=30`

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf *                |     746 |     0.127 |     170 |      - |  5869394.2
 pojobuf+str              |     746 |     0.132 |     176 |  1.036 |  5664388.8
 sajson                   |     746 |     0.120 |     161 |  0.947 |  6196013.3
 simdjson                 |     746 |     0.169 |     226 |  1.327 |  4424673.8
 boost                    |     746 |     0.189 |     252 |  1.484 |  3955461.3
 nlohmann                 |     746 |     0.396 |     531 |  3.118 |  1882412.3
 pojobuf *                |    2000 |     1.468 |     734 |      - |  1362397.8
 pojobuf+str              |    2000 |     1.427 |     713 |  0.972 |  1401541.7
 sajson                   |    2000 |     1.501 |     750 |  1.023 |  1332356.3
 simdjson                 |    2000 |     1.522 |     760 |  1.037 |  1314146.8
 boost                    |    2000 |     2.106 |    1053 |  1.435 |   949667.6
 nlohmann                 |    2000 |     7.258 |    3628 |  4.944 |   275565.6

```
> date
The current date is: Wed 23 Sept 2026

> ver
Microsoft Windows [Version 10.0.26200.9457]

> powershell "(Get-CimInstance -ClassName Win32_Processor).Name"
AMD Ryzen 7 5800H with Radeon Graphics

❯ cl
Microsoft (R) C/C++ Optimizing Compiler Version 19.51.36257 for x64
```

## Json Parse

`> bin\bench-pojobuf-json-parse.exe --samples=30`

### canada.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     2.938 | 2938300 |      - |      340.3
 pojobuf-mut charconv     |       1 |    13.088 |13088200 |  4.454 |       76.4
 sajson-mut               |       1 |     3.484 | 3484400 |  1.186 |      287.0
 pojobuf-const            |       1 |     2.908 | 2908100 |  0.990 |      343.9
 sajson-const             |       1 |     3.479 | 3478700 |  1.184 |      287.5
 simdjson                 |       1 |     2.578 | 2578200 |  0.877 |      387.9
 boost                    |       1 |     5.479 | 5478600 |  1.865 |      182.5

### citm_catalog.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     1.185 | 1184800 |      - |      844.0
 pojobuf-mut charconv     |       1 |     1.063 | 1062900 |  0.897 |      940.8
 sajson-mut               |       1 |     1.342 | 1342400 |  1.133 |      744.9
 pojobuf-const            |       1 |     1.241 | 1241200 |  1.048 |      805.7
 sajson-const             |       1 |     1.431 | 1431500 |  1.208 |      698.6
 simdjson                 |       1 |     0.830 |  830400 |  0.701 |     1204.2
 boost                    |       1 |     1.971 | 1971000 |  1.664 |      507.4

### gsoc-2018.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     2.103 | 2103400 |      - |      475.4
 pojobuf-mut charconv     |       1 |     2.686 | 2686000 |  1.277 |      372.3
 sajson-mut               |       1 |     2.289 | 2288800 |  1.088 |      436.9
 pojobuf-const            |       1 |     3.254 | 3254200 |  1.547 |      307.3
 sajson-const             |       1 |     2.695 | 2694500 |  1.281 |      371.1
 simdjson                 |       1 |     1.272 | 1271800 |  0.605 |      786.3
 boost                    |       1 |     2.405 | 2404900 |  1.143 |      415.8

### marine_ik.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     3.970 | 3969900 |      - |      251.9
 pojobuf-mut charconv     |       1 |    10.528 |10528200 |  2.652 |       95.0
 sajson-mut               |       1 |     4.953 | 4953400 |  1.248 |      201.9
 pojobuf-const            |       1 |     4.081 | 4081100 |  1.028 |      245.0
 sajson-const             |       1 |     5.207 | 5207300 |  1.312 |      192.0
 simdjson                 |       1 |     3.818 | 3818200 |  0.962 |      261.9
 boost                    |       1 |     9.298 | 9298000 |  2.342 |      107.6

### mesh.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     1.107 | 1106600 |      - |      903.7
 pojobuf-mut charconv     |       1 |     3.487 | 3486600 |  3.151 |      286.8
 sajson-mut               |       1 |     1.319 | 1319100 |  1.192 |      758.1
 pojobuf-const            |       1 |     1.088 | 1088000 |  0.983 |      919.1
 sajson-const             |       1 |     1.342 | 1341900 |  1.213 |      745.2
 simdjson                 |       1 |     0.982 |  982100 |  0.887 |     1018.2
 boost                    |       1 |     2.367 | 2367100 |  2.139 |      422.5

### mesh.pretty.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     1.578 | 1577600 |      - |      633.9
 pojobuf-mut charconv     |       1 |     4.199 | 4199300 |  2.662 |      238.1
 sajson-mut               |       1 |     1.705 | 1705200 |  1.081 |      586.4
 pojobuf-const            |       1 |     1.520 | 1519900 |  0.963 |      657.9
 sajson-const             |       1 |     1.905 | 1904500 |  1.207 |      525.1
 simdjson                 |       1 |     1.270 | 1269700 |  0.805 |      787.6
 boost                    |       1 |     2.450 | 2449800 |  1.553 |      408.2

## POJO Travese

`> bin\bench-pojobuf-traverse.exe --samples=30`

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf *                |     746 |     0.126 |     168 |      - |  5939490.4
 pojobuf+str              |     746 |     0.128 |     171 |  1.021 |  5814497.3
 sajson                   |     746 |     0.134 |     180 |  1.069 |  5554728.2
 simdjson                 |     746 |     0.158 |     211 |  1.259 |  4718532.6
 boost                    |     746 |     3.283 |    4400 | 26.138 |   227238.1
 pojobuf *                |    2000 |     1.464 |     731 |      - |  1366400.2
 pojobuf+str              |    2000 |     1.388 |     693 |  0.948 |  1441233.7
 sajson                   |    2000 |     1.954 |     976 |  1.335 |  1023803.4
 simdjson                 |    2000 |     1.440 |     720 |  0.984 |  1388792.4
 boost                    |    2000 |    19.117 |    9558 | 13.061 |   104619.5

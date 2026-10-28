```
$ date
Wed Oct 28

$ inxi
CPU: 24-core Intel Core Ultra 9 285K (-MCP-)
speed/min/max: 800/800/5800:5600:4600 MHz
Kernel: 7.0.0-38-generic x86_64

$ gcc --version
gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
```

## Json Parse

`$ ./bin/bench-pojobuf-json-parse --samples=30`

### canada.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     1.313 | 1312895 |      - |      761.7
 pojobuf-mut charconv     |       1 |     1.489 | 1489229 |  1.134 |      671.5
 sajson-mut               |       1 |     1.433 | 1433235 |  1.092 |      697.7
 pojobuf-const            |       1 |     1.307 | 1307461 |  0.996 |      764.8
 sajson-const             |       1 |     1.478 | 1477992 |  1.126 |      676.6
 simdjson                 |       1 |     1.291 | 1290629 |  0.983 |      774.8
 boost                    |       1 |     1.675 | 1674748 |  1.276 |      597.1
 nlohmann                 |       1 |    13.972 |13972250 | 10.642 |       71.6

### citm_catalog.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     0.529 |  528540 |      - |     1892.0
 pojobuf-mut charconv     |       1 |     0.516 |  515548 |  0.975 |     1939.7
 sajson-mut               |       1 |     0.540 |  540476 |  1.023 |     1850.2
 pojobuf-const            |       1 |     0.558 |  557644 |  1.055 |     1793.3
 sajson-const             |       1 |     0.589 |  588872 |  1.114 |     1698.2
 simdjson                 |       1 |     0.264 |  264266 |  0.500 |     3784.1
 boost                    |       1 |     0.590 |  590316 |  1.117 |     1694.0
 nlohmann                 |       1 |     4.432 | 4431979 |  8.385 |      225.6

### gsoc-2018.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     0.985 |  984554 |      - |     1015.7
 pojobuf-mut charconv     |       1 |     0.997 |  996747 |  1.012 |     1003.3
 sajson-mut               |       1 |     0.994 |  994444 |  1.010 |     1005.6
 pojobuf-const            |       1 |     1.215 | 1215048 |  1.234 |      823.0
 sajson-const             |       1 |     1.112 | 1112250 |  1.130 |      899.1
 simdjson                 |       1 |     0.469 |  469216 |  0.477 |     2131.2
 boost                    |       1 |     0.785 |  785008 |  0.797 |     1273.9
 nlohmann                 |       1 |     9.974 | 9973752 | 10.130 |      100.3

### marine_ik.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     1.676 | 1676022 |      - |      596.7
 pojobuf-mut charconv     |       1 |     2.163 | 2162818 |  1.290 |      462.4
 sajson-mut               |       1 |     1.875 | 1874566 |  1.118 |      533.5
 pojobuf-const            |       1 |     1.699 | 1698977 |  1.014 |      588.6
 sajson-const             |       1 |     1.967 | 1966609 |  1.173 |      508.5
 simdjson                 |       1 |     1.572 | 1572360 |  0.938 |      636.0
 boost                    |       1 |     2.284 | 2284235 |  1.363 |      437.8
 nlohmann                 |       1 |    14.711 |14710546 |  8.777 |       68.0

### mesh.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     0.397 |  397213 |      - |     2517.5
 pojobuf-mut charconv     |       1 |     0.509 |  509101 |  1.282 |     1964.2
 sajson-mut               |       1 |     0.492 |  492003 |  1.239 |     2032.5
 pojobuf-const            |       1 |     0.400 |  399934 |  1.007 |     2500.4
 sajson-const             |       1 |     0.510 |  510207 |  1.284 |     1960.0
 simdjson                 |       1 |     0.400 |  400282 |  1.008 |     2498.2
 boost                    |       1 |     0.716 |  716288 |  1.803 |     1396.1
 nlohmann                 |       1 |     3.955 | 3954859 |  9.957 |      252.9

### mesh.pretty.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     0.718 |  717749 |      - |     1393.2
 pojobuf-mut charconv     |       1 |     0.770 |  770496 |  1.073 |     1297.9
 sajson-mut               |       1 |     0.778 |  777867 |  1.084 |     1285.6
 pojobuf-const            |       1 |     0.722 |  722403 |  1.006 |     1384.3
 sajson-const             |       1 |     0.842 |  841641 |  1.173 |     1188.2
 simdjson                 |       1 |     0.540 |  539970 |  0.752 |     1852.0
 boost                    |       1 |     0.832 |  832265 |  1.160 |     1201.5
 nlohmann                 |       1 |     5.684 | 5683501 |  7.919 |      175.9

## POJO Traverse

`$ ./bin/bench-pojobuf-traverse --samples=30`

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf *                |     746 |     0.024 |      32 |      - | 30582544.2
 pojobuf+str              |     746 |     0.027 |      35 |  1.087 | 28137140.3
 sajson                   |     746 |     0.031 |      41 |  1.281 | 23877348.5
 simdjson                 |     746 |     0.077 |     103 |  3.167 |  9657208.0
 boost                    |     746 |     0.056 |      74 |  2.283 | 13396304.3
 nlohmann                 |     746 |     0.104 |     139 |  4.259 |  7180879.2
 pojobuf *                |    2000 |     0.830 |     414 |      - |  2411035.8
 pojobuf+str              |    2000 |     0.828 |     413 |  0.998 |  2416337.3
 sajson                   |    2000 |     0.844 |     421 |  1.017 |  2370831.2
 simdjson                 |    2000 |     1.196 |     597 |  1.442 |  1672299.5
 boost                    |    2000 |     1.039 |     519 |  1.253 |  1924729.6
 nlohmann                 |    2000 |     1.835 |     917 |  2.212 |  1089870.7

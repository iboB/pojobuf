```
$ date
Wed Sep 23

$ inxi
CPU: 24-core Intel Core Ultra 9 285K (-MCP-)
speed/min/max: 800/800/5800:5600:4600 MHz Kernel: 7.0.0-31-generic x86_64

$ gcc --version
gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0
```

## Json Parse

`$ ./bin/bench-pojobuf-json-parse --samples=30`

### canada.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     1.375 | 1375453 |      - |      727.0
 pojobuf-mut charconv     |       1 |     1.561 | 1561017 |  1.135 |      640.6
 sajson-mut               |       1 |     1.336 | 1336310 |  0.972 |      748.3
 pojobuf-const            |       1 |     1.357 | 1357258 |  0.987 |      736.8
 sajson-const             |       1 |     1.534 | 1534382 |  1.116 |      651.7
 simdjson                 |       1 |     1.282 | 1282399 |  0.932 |      779.8
 boost                    |       1 |     1.625 | 1624693 |  1.181 |      615.5

### citm_catalog.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     0.520 |  519710 |      - |     1924.2
 pojobuf-mut charconv     |       1 |     0.525 |  524925 |  1.010 |     1905.0
 sajson-mut               |       1 |     0.545 |  544711 |  1.048 |     1835.8
 pojobuf-const            |       1 |     0.517 |  516845 |  0.994 |     1934.8
 sajson-const             |       1 |     0.584 |  583552 |  1.123 |     1713.6
 simdjson                 |       1 |     0.270 |  270434 |  0.520 |     3697.8
 boost                    |       1 |     0.592 |  592140 |  1.139 |     1688.8

### gsoc-2018.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     0.996 |  995935 |      - |     1004.1
 pojobuf-mut charconv     |       1 |     0.999 |  998912 |  1.003 |     1001.1
 sajson-mut               |       1 |     0.972 |  971658 |  0.976 |     1029.2
 pojobuf-const            |       1 |     1.180 | 1180478 |  1.185 |      847.1
 sajson-const             |       1 |     1.120 | 1120133 |  1.125 |      892.8
 simdjson                 |       1 |     0.466 |  465542 |  0.467 |     2148.0
 boost                    |       1 |     0.784 |  784148 |  0.787 |     1275.3

### marine_ik.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     1.757 | 1757402 |      - |      569.0
 pojobuf-mut charconv     |       1 |     2.351 | 2351423 |  1.338 |      425.3
 sajson-mut               |       1 |     1.869 | 1869142 |  1.064 |      535.0
 pojobuf-const            |       1 |     1.784 | 1783598 |  1.015 |      560.7
 sajson-const             |       1 |     1.959 | 1959139 |  1.115 |      510.4
 simdjson                 |       1 |     1.549 | 1548940 |  0.881 |      645.6
 boost                    |       1 |     2.343 | 2342820 |  1.333 |      426.8

### mesh.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     0.441 |  440534 |      - |     2270.0
 pojobuf-mut charconv     |       1 |     0.550 |  550390 |  1.249 |     1816.9
 sajson-mut               |       1 |     0.500 |  500024 |  1.135 |     1999.9
 pojobuf-const            |       1 |     0.437 |  437146 |  0.992 |     2287.6
 sajson-const             |       1 |     0.503 |  502994 |  1.142 |     1988.1
 simdjson                 |       1 |     0.388 |  388212 |  0.881 |     2575.9
 boost                    |       1 |     0.737 |  737351 |  1.674 |     1356.2

### mesh.pretty.json:

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf-mut *            |       1 |     0.745 |  744508 |      - |     1343.2
 pojobuf-mut charconv     |       1 |     0.857 |  856766 |  1.151 |     1167.2
 sajson-mut               |       1 |     0.784 |  784172 |  1.053 |     1275.2
 pojobuf-const            |       1 |     0.722 |  721788 |  0.969 |     1385.4
 sajson-const             |       1 |     0.836 |  835582 |  1.122 |     1196.8
 simdjson                 |       1 |     0.530 |  530108 |  0.712 |     1886.4
 boost                    |       1 |     0.841 |  840773 |  1.129 |     1189.4

## POJO Traverse

`$ ./bin/bench-pojobuf-traverse --samples=30`

 Name (* = baseline)      |   Dim   |  Total ms |  ns/op  |Baseline| Ops/second
--------------------------|--------:|----------:|--------:|-------:|----------:
 pojobuf *                |     746 |     0.025 |      33 |      - | 29930990.2
 pojobuf+str              |     746 |     0.026 |      34 |  1.046 | 28613071.5
 sajson                   |     746 |     0.028 |      37 |  1.116 | 26815240.8
 simdjson                 |     746 |     0.071 |      95 |  2.865 | 10446131.0
 boost                    |     746 |     0.854 |    1144 | 34.259 |   873656.0
 pojobuf *                |    2000 |     0.830 |     414 |      - |  2410300.7
 pojobuf+str              |    2000 |     0.828 |     414 |  0.998 |  2414368.4
 sajson                   |    2000 |     0.828 |     414 |  0.998 |  2414718.2
 simdjson                 |    2000 |     1.178 |     588 |  1.419 |  1698487.8
 boost                    |    2000 |     8.976 |    4487 | 10.817 |   222824.2

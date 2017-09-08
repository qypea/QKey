To build test programs use cmake:

```shell
cmake . -DSPRITZ_LIB=~/docs/programs/Arduino/libraries/SpritzCipher
make
```

Two test programs will be built:
* test_huff: Validates huffman compression and decompression routines work
* extract: Extracts the contents of a database backup

```shell
./build/extract PASSWD.DB
```

```
Database file demo.db
Password? test
Records: 2
File check: .1xopy+0J1[NOwz[
Record 0
  Description: test 1
  Username: foo
  Separator: t
  Password: B5T3eib92VKNdz0(
Record 1
  Description: test 2
  Username: bar
  Separator: n
  Password: wr6N8ZFksC4&8kLB
```

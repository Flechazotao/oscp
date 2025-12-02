Lab4 基本文件系统扩展
2025.12.2

lab4：
  在OpenFile类中，增加WriteBack方法，手动调用此方法写回文件头。

  FileHeader的构造函数中增加了：
    memset(dataSectors, 0, sizeof(dataSectors));


测试命令行：
reset
rm -f DISK
./nachos -f
./nachos -cp test/big big
./nachos -D
ls --full-time test/big    # 对比检查Nachos文件big的修改时间应与test/big相同
./nachos -ap test/small big
./nachos -D    # 包括检查Nachos文件big的修改时间应为当前时间
./nachos -hap test/small big
./nachos -D
./nachos -ap test/small small
./nachos -nap small small2
./nachos -D    # 包括检查Nachos文件small及small2的修改时间应不是当前时间
./nachos -nap small small2
./nachos -D    # 包括检查Nachos文件small2的修改时间应为当前时间


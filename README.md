# oscp
操作系统课设代码文件
导入并安装环境命令行操作:
git clone https://github.com/Flechazotao/oscp
sudo apt update
sudo apt install g++
将压缩包 gcc-2.8.1-mips.tar.gz 复制到 ~ (Home，用户主目录)
cd /usr/local
sudo tar -xzvf ~/gcc-2.8.1-mips.tar.gz
这样就安装好了用于MIPS的交叉编译器
安装 Nachos 3.4
cd ~
mkdir oscp
cd oscp
将压缩包 nachos-3.4-ualr-2022.tar.gz 复制到 ~/oscp
tar -xzvf nachos-3.4-ualr-2022.tar.gz
测试Nachos threads
cd ~/oscp/nachos-3.4-ualr-2022/code/threads
make clean
make
./nachos

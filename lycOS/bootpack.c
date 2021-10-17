
// 调用汇编中 HALT
void io_hlt();

// 系统入口
void MyOSMain(){

    fin:
    io_hlt();
    goto fin;
}

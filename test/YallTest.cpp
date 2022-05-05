//
// Created by gloom on 2022/5/5.
//

#include<iostream>
#include "Yall.h"

int main() {
    YALL_DEBUG_ << "This is DEBUG Message";
    YALL_INFO_ << "This is INFO Message";
    YALL_WARN_ << "This is Warning Message";
    YALL_ERROR_ << "This is error Message";
    YALL_CRITICAL_ << "This is Critical Message";
    return 0;
}

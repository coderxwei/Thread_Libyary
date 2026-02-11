#pragma once

// 线程池工作模式枚举
enum class PoolMode
{
    FIXED,   // 固定线程数量模式
    CACHED   // 动态线程数量模式
};

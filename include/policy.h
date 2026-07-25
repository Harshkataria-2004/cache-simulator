#pragma once

enum class ReplacementPolicy {
    LRU,
    FIFO,
    LFU,
    RANDOM
};

enum class WritePolicy {
    WRITE_BACK,
    WRITE_THROUGH
};

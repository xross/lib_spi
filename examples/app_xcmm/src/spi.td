include "interface.td"

def : Interface<"spi", [
    Method<"set", Void, [Int]>,
    Method<"get", Int, [Void]>,
    Method<"triple", Void, [Void]>,
    Method<"add_all", Void, [Unsigned, Short, Char]>,
    Method<"stop", Void, [Void]>,

    //device_index, speed_in_khz, spi_mode_t mode
    Method<"begin_transaction", Void, [Unsigned, Unsigned, Unsigned]>,

    // ss_deassrt_time
    Method<"end_transaction", Void, [Unsigned]>,

    // data
    Method<"transfer8", Char, [Char]>,
    Method<"transfer32", Unsigned, [Unsigned]>,

]>;


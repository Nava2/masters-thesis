-- Memory mapped machine

-- Many irrelevant configuration parameters are
-- ommitted to reduce this listing size.

local A = Register.new {
    width = 8,
};

local B = Register.new {
    width = 8,
};

local X = Register.new {
    width = 16,
};

return {
    proc = Proc.new {
        name = "memory-mapped",
        clock = Clock.new {
            name = "cpu",
            period = millis(1),
        },

        -- Registers within the CPU
        registers = {
            A = A,
            B = B,
            X = X
        },

        -- Memory map for addressable registers
        memory = {
            0x0000 = A,
            0x0001 = B,
            0x0002 = X, -- two words wide
            0x0004 = Memory.new {
                width = 8,
                length = 16 * 1024, -- 16k
            },
        }
    }
}
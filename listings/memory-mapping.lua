-- Memory mapped machine

local A = Register.new {
    width = 8,
    -- omitted parameters
};

local B = Register.new {
    width = 8,
    -- omitted parameters
};

local X = Register.new {
    width = 16,
    -- omitted parameters
};

return {
    proc = Proc.new {
        name = "memor-mapped",
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

        memory = {
            0x0000 = A,
            0x0001 = B,
            0x0002 = X,
            0x0004 = Memory.new {
                width = 8,
                length = 16 * 1024, -- 16k
            },
        }
    }
}
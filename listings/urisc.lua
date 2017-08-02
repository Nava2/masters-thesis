-- URISC Machine defined in procsim
-- \cite{Mavaddat1988}

-- variable defined for convenience
local wordWidth = 16

return {
    proc = Proc.new {
        name = "urisc",
        clock = Clock.new {
            name = "cpu",
            period = millis(1),
        },

        -- Registers within the CPU
        registers = {
            PC = Register.new {
                width = wordWidth,
                access = Access.ReadWrite,
                clock = "cpu", -- Previously defined
                readCount = 1,
                writeCount = 1
            },
            -- Buffer for ALU results
            R = Register.new {
                width = wordWidth,
                access = Access.ReadWrite,
                clock = "cpu",
                readCount = 1,
                writeCount = 1
            },
        },
        -- Define a memory mapping
        memory = {
            0x0000 = Memory.new {
                width = wordWidth,
                length = 16 * 1024, -- 16Ki
                access = Access.ReadWrite,
                clock = "cpu",
                readCount = 1,
                writeCount = 1
            },
        }
        -- Instruction table specification
        instructions = {
            -- SUBLEQ
            SUBLEQ = Instruction.new {
                -- AAAA BBBB PPPP
                code = Code.new(0, -- no constant values
                    {
                        A = u16(15), -- A operand
                    }),

                -- Word width is 16, so operands need to be
                -- defined outside the opcode
                op = {
                    B = u16(31), -- B operand
                    P = u16(15)  -- JUMP address
                },

                -- Function to execute
                exec = function (proc, operand)
                    local B = operand:B -- alias

                    -- Store A into "rhs" of operation
                    proc:R = operand:A
                    proc:memory[B] = B - proc:R  -- *B <- B - A
                    if (proc:N) { -- Negative val from ALU
                        proc:PC = operand:P -- Jump as result was < 0
                    } else {
                        proc:PC++ -- Next ins
                    }
                end
            }
        }
    } -- end Proc.new{}
}
-- Define the pairings for registers utilized later
-- instead of defining them individually as constants
local reg_pairs = {
  0x111 = "A",
  0x000 = "B",
  0x001 = "C",
  0x010 = "D",
  0x011 = "E",
  0x100 = "F",
  0x101 = "G"
}

-- Build up the table of registers, utilizing the `reg_pair` map defined
-- we can programmatically define registers.
-- Bind the clock `RegisterClock` to each
local reg_table = {}
for code, name in ipairs(reg_pairs) do
  reg_table[name] = Register.new {
    name = name,
    width = 8,    -- Width in bits
    access = Access.ReadWrite,
    clock = "RegisterClock",
    readCount = 1, -- Cycles req. read a value
    writeCount = 1 -- Cycles req. write a value
  }
end

local proc = Proc.new {

  name = "x86_sample",

  -- Define the master clock
  clock = Clock.new {
    name = "cpu",
    period = millis(1),
    type = PulseType.INV_PULSE
  },

  clocks = {
    -- Clock for the Registers
    Clock.new {
        name = "RegisterClock",
        freq = MHz(1),
        type = PulseType.PULSE
    }
  }

  -- Registers available to the processor
  registers = reg_table,

  -- Look-Up-Table of instructions, keyed by instruction name
  instructions = {
    -- Mov F, T  -- move F into T
    MOV = Instruction.new {
      -- Define the encoding for MOV
      -- 0b01 TTT FFF
      code = Code.new(0x40, -- 0100 0000
        {
            to = u3(5),   -- (unsigned 3-bits, index 5)
            from = u3(2)  -- (unsigned 3-bits, index 2)
        }),

      -- No operands required as the registers are encoded
      -- op = nil,

      -- Execution of the instruction
      exec = function (proc, operand)
          -- no cycles, convenience variable
          local from = proc.regs[operand.from]
          local to = proc.regs[operand.to]

          -- Read `from` into `to`
          to = from
      end
    },

    -- ADD R  -- Add value of `R` into `D`
    ADD = Instruction.new {
      -- Specify the encoding of ADD
      -- 0b10 000 RRR
      code = Code.new(0x80,  -- 1000 0000
        {
            register = u3(2) -- Register
        }),

      -- No operand
      -- op = nil,

      -- Function to execute
      exec = function (proc, operand)
          -- no cycles, convenience variable
        local reg = proc.regs[operand.register]

        -- Read `D`, write `reg`
        proc.regs.D = proc.regs.D + reg
      end
    }
  }
}

return {
  proc = proc
}
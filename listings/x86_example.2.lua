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

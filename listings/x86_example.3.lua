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
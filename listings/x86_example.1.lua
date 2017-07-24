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

-- Define a new processor
local proc = Proc.new {

  name = "x86_sample",

  -- Define the master clock
  clock = Clock.new {
    name = "cpu",
    period = millis(1),
    type = PulseType.INV_PULSE
  },

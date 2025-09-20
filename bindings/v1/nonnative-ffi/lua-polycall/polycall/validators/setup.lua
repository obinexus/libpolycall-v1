-- Setup validation module
local setup = {}

function setup.validate_environment()
    local marker_file = ".polycall-lua-setup-complete"
    local file = io.open(marker_file, "r")
    if file then
        file:close()
        return true
    end
    return false
end

return setup

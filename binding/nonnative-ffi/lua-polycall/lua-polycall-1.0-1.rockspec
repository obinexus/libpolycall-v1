rockspec_format = "3.0"
package = "lua-polycall"
version = "1.0-1"

source = {
   url = "git+https://github.com/obinexus/libpolycall-v1trial.git",
   dir = "bindings/lua-polycall"
}

description = {
   summary = "Protocol-compliant Lua binding for LibPolyCall",
   detailed = [[
      LibPolyCall Trial v1 Lua Adapter Binding - Protocol-compliant adapter for polycall.exe runtime.
      
      CRITICAL PROTOCOL COMPLIANCE NOTICE:
      This binding acts as an ADAPTER for the LibPolyCall runtime system. All execution must flow 
      through the polycall.exe runtime following the program-first architecture paradigm.
      
      Key Features:
      - Runtime Dependency: Requires polycall.exe runtime for all operations
      - Adapter Pattern: Never bypasses protocol validation layer  
      - Zero-Trust Architecture: Cryptographic validation at every state transition
      - State Machine Binding: All interactions follow finite automaton patterns
      - Telemetry Integration: Silent protocol observation for debugging
      
      This module DOES NOT execute user code directly. All logic execution occurs within 
      the polycall.exe binary. The Lua binding provides interface translation while 
      maintaining strict protocol compliance with LibPolyCall Trial v1 specification.
   ]],
   homepage = "https://github.com/obinexus/libpolycall-v1trial",
   issues_url = "https://github.com/obinexus/libpolycall-v1trial/issues",
   license = "MIT",
   maintainer = "Nnamdi Michael Okpala <obinexuscomputing@gmail.com>"
}

dependencies = {
   "lua >= 5.3, < 5.5",
   "luasocket >= 3.0",
   "luasec >= 1.0",
   "lua-cjson >= 2.1"
}

build = {
   type = "builtin",
   
   install = {
      lua = {
         -- Core adapter modules
         ["polycall.core.binding"] = "polycall/core/binding.lua",
         ["polycall.core.protocol"] = "polycall/core/protocol.lua",
         ["polycall.core.state"] = "polycall/core/state.lua",
         ["polycall.core.telemetry"] = "polycall/core/telemetry.lua",
         ["polycall.core.auth"] = "polycall/core/auth.lua",
         
         -- Configuration management
         ["polycall.config.manager"] = "polycall/config/manager.lua",
         ["polycall.config.validator"] = "polycall/config/validator.lua",
         
         -- CLI framework
         ["polycall.cli.main"] = "polycall/cli/main.lua",
         ["polycall.cli.commands.info"] = "polycall/cli/commands/info.lua",
         ["polycall.cli.commands.test"] = "polycall/cli/commands/test.lua",
         ["polycall.cli.commands.telemetry"] = "polycall/cli/commands/telemetry.lua",
         ["polycall.cli.registry"] = "polycall/cli/registry.lua",
         
         -- Utilities and validation
         ["polycall.utils.logger"] = "polycall/utils/logger.lua",
         ["polycall.utils.validator"] = "polycall/utils/validator.lua",
         ["polycall.utils.crypto"] = "polycall/utils/crypto.lua",
         
         -- Exception handling
         ["polycall.exceptions.protocol"] = "polycall/exceptions/protocol.lua",
         ["polycall.exceptions.connection"] = "polycall/exceptions/connection.lua",
         
         -- Validators
         ["polycall.validators.setup"] = "polycall/validators/setup.lua",
         ["polycall.validators.runtime"] = "polycall/validators/runtime.lua",
         ["polycall.validators.ssh"] = "polycall/validators/ssh.lua",
         
         -- Main module
         ["polycall"] = "polycall/init.lua"
      },
      
      bin = {
         ["lua-polycall"] = "bin/lua-polycall"
      },
      
      conf = {
         ["lua-polycall.conf"] = "config/lua-polycall.conf"
      }
   }
}

test = {
   type = "busted",
   flags = { "--verbose", "--coverage" },
   modules = {
      "spec.core.binding_spec",
      "spec.core.protocol_spec",
      "spec.core.state_spec",
      "spec.core.telemetry_spec",
      "spec.cli.main_spec",
      "spec.config.manager_spec",
      "spec.utils.validator_spec",
      "spec.integration.runtime_spec"
   }
}

test_dependencies = {
   "busted >= 2.0",
   "luassert >= 1.8",
   "luacov >= 0.15",
   "ldoc >= 1.4"
}


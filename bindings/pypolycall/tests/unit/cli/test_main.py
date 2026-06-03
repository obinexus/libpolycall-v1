"""
CLI Main Module Tests
"""

import pytest
import asyncio
from pypolycall.cli.main import CLI, main

class TestCLI:
    """Test CLI main functionality"""
    
    def test_cli_initialization(self):
        """Test CLI can be initialized"""
        cli = CLI()
        assert cli is not None
        assert hasattr(cli, 'parser')
    
    @pytest.mark.asyncio
    async def test_info_command(self):
        """Test info command execution"""
        cli = CLI()
        result = await cli.run(['info'])
        assert result == 0
    
    @pytest.mark.asyncio
    async def test_test_command(self):
        """Test test command execution"""
        cli = CLI()
        result = await cli.run(['test'])
        assert result == 0
    
    def test_parser_creation(self):
        """Test argument parser creation"""
        cli = CLI()
        parser = cli.parser
        assert parser is not None
        
        # Test help doesn't raise exception
        with pytest.raises(SystemExit):
            parser.parse_args(['--help'])

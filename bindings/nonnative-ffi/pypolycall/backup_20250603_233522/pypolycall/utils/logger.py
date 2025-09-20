"""
Logging utilities
"""

import logging
import structlog

class Logger:
    """Unified logging system"""
    
    _configured = False
    
    @classmethod
    def setup_logging(cls, level: str = "INFO") -> None:
        """Setup logging configuration"""
        if cls._configured:
            return
        
        logging.basicConfig(
            level=getattr(logging, level.upper()),
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        )
        
        cls._configured = True
    
    @classmethod
    def get_logger(cls, name: str):
        """Get logger instance"""
        if not cls._configured:
            cls.setup_logging()
        return logging.getLogger(name)

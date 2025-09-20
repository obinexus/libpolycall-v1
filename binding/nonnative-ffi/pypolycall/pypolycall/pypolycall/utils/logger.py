"""
Unified Logging System
Structured logging for PyPolyCall architecture
"""

import logging
import structlog
from typing import Any

class Logger:
    """
    Unified Logger with structured logging support
    
    RESPONSIBILITIES:
    - Consistent logging across all layers
    - Structured log formatting
    - Performance logging integration
    """
    
    _configured = False
    
    @classmethod
    def setup_logging(cls, level: str = "INFO") -> None:
        """Setup structured logging configuration"""
        if cls._configured:
            return
        
        # Configure structlog
        structlog.configure(
            processors=[
                structlog.stdlib.filter_by_level,
                structlog.stdlib.add_logger_name,
                structlog.stdlib.add_log_level,
                structlog.stdlib.PositionalArgumentsFormatter(),
                structlog.processors.TimeStamper(fmt="iso"),
                structlog.processors.StackInfoRenderer(),
                structlog.processors.format_exc_info,
                structlog.processors.UnicodeDecoder(),
                structlog.processors.JSONRenderer()
            ],
            context_class=dict,
            logger_factory=structlog.stdlib.LoggerFactory(),
            wrapper_class=structlog.stdlib.BoundLogger,
            cache_logger_on_first_use=True,
        )
        
        # Configure standard logging
        logging.basicConfig(
            level=getattr(logging, level.upper()),
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        )
        
        cls._configured = True
    
    @classmethod
    def get_logger(cls, name: str) -> Any:
        """Get structured logger instance"""
        if not cls._configured:
            cls.setup_logging()
        return structlog.get_logger(name)

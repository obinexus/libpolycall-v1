package org.obinexus.core;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import java.util.Map;
import java.util.concurrent.CompletableFuture;

/**
 * Protocol binding adapter for polycall.exe runtime
 * Implements zero-trust architecture with state machine compliance
 */
public class ProtocolBinding {
    private static final Logger logger = LoggerFactory.getLogger(ProtocolBinding.class);
    
    private final String polycallHost;
    private final int polycallPort;
    private final ProtocolHandler protocolHandler;
    private final StateManager stateManager;
    private final TelemetryObserver telemetryObserver;
    
    private boolean connected = false;
    private boolean authenticated = false;
    
    public ProtocolBinding(String host, int port) {
        this.polycallHost = host;
        this.polycallPort = port;
        this.protocolHandler = new ProtocolHandler(host, port);
        this.stateManager = new StateManager();
        this.telemetryObserver = new TelemetryObserver();
        
        logger.info("Initialized ProtocolBinding for {}:{}", host, port);
    }
    
    public CompletableFuture<Boolean> connect() {
        return CompletableFuture.supplyAsync(() -> {
            try {
                logger.debug("Initiating connection to polycall.exe runtime");
                stateManager.transitionTo(StateManager.State.HANDSHAKE);
                
                boolean result = protocolHandler.establishConnection();
                if (result) {
                    connected = true;
                    stateManager.transitionTo(StateManager.State.AUTH);
                    telemetryObserver.recordEvent("connection_established");
                }
                
                return result;
            } catch (Exception e) {
                logger.error("Connection failed", e);
                stateManager.transitionTo(StateManager.State.ERROR);
                return false;
            }
        });
    }
    
    public CompletableFuture<Boolean> authenticate(Map<String, Object> credentials) {
        return CompletableFuture.supplyAsync(() -> {
            try {
                if (!connected) {
                    throw new IllegalStateException("Must connect before authentication");
                }
                
                logger.debug("Performing zero-trust authentication");
                boolean result = protocolHandler.authenticate(credentials);
                
                if (result) {
                    authenticated = true;
                    stateManager.transitionTo(StateManager.State.READY);
                    telemetryObserver.recordEvent("authentication_success");
                }
                
                return result;
            } catch (Exception e) {
                logger.error("Authentication failed", e);
                stateManager.transitionTo(StateManager.State.ERROR);
                return false;
            }
        });
    }
    
    public CompletableFuture<Object> executeOperation(String operation, Map<String, Object> params) {
        return CompletableFuture.supplyAsync(() -> {
            try {
                if (!authenticated) {
                    throw new IllegalStateException("Must authenticate before operation execution");
                }
                
                stateManager.transitionTo(StateManager.State.EXECUTING);
                telemetryObserver.recordEvent("operation_start", operation);
                
                Object result = protocolHandler.executeOperation(operation, params);
                
                stateManager.transitionTo(StateManager.State.READY);
                telemetryObserver.recordEvent("operation_complete", operation);
                
                return result;
            } catch (Exception e) {
                logger.error("Operation execution failed: {}", operation, e);
                stateManager.transitionTo(StateManager.State.ERROR);
                throw new RuntimeException("Operation failed: " + operation, e);
            }
        });
    }
    
    public CompletableFuture<Void> shutdown() {
        return CompletableFuture.runAsync(() -> {
            try {
                logger.debug("Shutting down protocol binding");
                protocolHandler.disconnect();
                stateManager.transitionTo(StateManager.State.SHUTDOWN);
                telemetryObserver.recordEvent("shutdown_complete");
            } catch (Exception e) {
                logger.error("Shutdown error", e);
            }
        });
    }
    
    // Getters
    public boolean isConnected() { return connected; }
    public boolean isAuthenticated() { return authenticated; }
    public String getRuntimeVersion() { return protocolHandler.getRuntimeVersion(); }
    public TelemetryObserver getTelemetry() { return telemetryObserver; }
}

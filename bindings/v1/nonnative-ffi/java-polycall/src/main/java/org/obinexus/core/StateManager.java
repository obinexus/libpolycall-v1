package org.obinexus.core;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * State machine synchronization for protocol compliance
 */
public class StateManager {
    private static final Logger logger = LoggerFactory.getLogger(StateManager.class);
    
    public enum State {
        INIT, HANDSHAKE, AUTH, READY, EXECUTING, ERROR, SHUTDOWN
    }
    
    private State currentState = State.INIT;
    
    public synchronized void transitionTo(State newState) {
        logger.debug("State transition: {} -> {}", currentState, newState);
        this.currentState = newState;
    }
    
    public State getCurrentState() {
        return currentState;
    }
    
    public boolean isValidTransition(State from, State to) {
        // Define valid state transitions according to LibPolyCall protocol
        return switch (from) {
            case INIT -> to == State.HANDSHAKE || to == State.ERROR;
            case HANDSHAKE -> to == State.AUTH || to == State.ERROR;
            case AUTH -> to == State.READY || to == State.ERROR;
            case READY -> to == State.EXECUTING || to == State.SHUTDOWN || to == State.ERROR;
            case EXECUTING -> to == State.READY || to == State.ERROR;
            case ERROR -> to == State.SHUTDOWN;
            case SHUTDOWN -> false; // Terminal state
        };
    }
}

from abc import ABC, abstractmethod

class Observer(ABC):
    @abstractmethod
    def notify(self, ticket_id: str, number: str):
        pass

class PhoneRift:
    def __init__(self, schema):
        self.schema = schema
        self.observers = []

    def add_observer(self, obs: Observer):
        self.observers.append(obs)

    def route_call(self, raw_input: str, ticket_id: str):
        number = self.schema.match(raw_input)
        if number:
            for obs in self.observers:
                obs.notify(ticket_id, number)  # rift open: call the conjugate service
            return f"Routed {ticket_id} to {number}"  # stage2 success
        return "REJECT: 00-veto"  # invariant fail

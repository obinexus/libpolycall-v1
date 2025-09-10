// adapter/DOPAdapter.js
class DOPAdapter {
    constructor(logic) {
      this.logic = logic;
    }
  
    toFunctional() {
      const { name, state, actions, render } = this.logic;
      return function FunctionalComponent() {
        let currentState = { ...state };
        const ctx = {
          state: currentState,
          ...actions
        };
        return render(ctx);
      };
    }
  
    toOOP() {
      const { name, state, actions, render } = this.logic;
      return class OOPComponent {
        constructor() {
          this.state = { ...state };
          Object.keys(actions).forEach((key) => {
            this[key] = actions[key].bind(null, this);
          });
        }
  
        render() {
          return render(this);
        }
      };
    }
  }
  
  module.exports = DOPAdapter;
  
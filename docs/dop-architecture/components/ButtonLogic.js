// components/ButtonLogic.js
module.exports = {
    name: "Button",
    state: { clicked: false },
    actions: {
      toggle: (ctx) => {
        ctx.state.clicked = !ctx.state.clicked;
      }
    },
    render: (ctx) => {
      return `<button>${ctx.state.clicked ? "ON" : "OFF"}</button>`;
    }
  };
  
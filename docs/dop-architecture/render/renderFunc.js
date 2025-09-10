// render/renderFunc.js
module.exports = (Component) => {
    const instance = Component();
    console.log("[Functional Render]:", instance);
  };
  
// render/renderOOP.js
module.exports = (ComponentClass) => {
    const instance = new ComponentClass();
    console.log("[OOP Render]:", instance.render());
  };
  
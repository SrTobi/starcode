var spritesheet = require('spritesheet-js');

var options = {
    format: 'pixi.js',
    powerOfTwo: true,
    trim: false,
    name: "tiles",
    path: "assets"
};

spritesheet('assets/tiles/*.png', options, function (err) {
  if (err) throw err;

  console.log('spritesheet successfully generated');
});

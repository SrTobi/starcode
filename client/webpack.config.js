var webpack = require('webpack');
var path = require('path');
var HtmlWebpackPlugin = require('html-webpack-plugin');
var SplitByPathPlugin = require('webpack-split-by-path');

var r = file => path.resolve(__dirname, file[0]);

var config = {
  entry: r`src/index.tsx`,
  output: {
    path: r`dist`,
    filename: "[name]-[hash].js",
    chunkFilename: "[name]-[hash].js"
  },
  devtool: 'source-map',
  resolve: {
    extensions: [
      '',
      '.webpack.js',
      '.web.js',
      '.ts',
      '.tsx',
      '.js'
    ]
  },
  module: {
    loaders: [
      { test: /\.css$/, loader: "style-loader!css-loader" },
      { test: /\.(jpe?g|png|gif|woff|woff2|eot|ttf|svg)$/i, loader: "file" },
      { test: /\.tsx?$/, loader: 'ts-loader' }
    ],
    preLoaders: [
      /*
       * bundled js ---[sourcemap 1]--> js ---[sourcemap 2]--> ts
       * The source-map-loader combines both sourcemaps.
       */
      { test: /\.js$/, loader: "source-map-loader" }
    ]
  },
  plugins: [

		new webpack.ProvidePlugin({
        $: "jquery",
        jQuery: "jquery",
        "window.jQuery": "jquery"
    }),


    // generates two bundles: One for ./src code, one for node_modules code.
    new SplitByPathPlugin([{ name: 'node_modules-bundle', path: r`node_modules` }],
      { manifest: 'app-entry' }
    ),

    // generates an index.html
    new HtmlWebpackPlugin({
      title: "StarCode"
    })
  ]
};

module.exports = config;


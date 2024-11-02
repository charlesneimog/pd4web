let handPose;
let hands = [];
var p5jsConfigured = false;
let video;
let canvas;

function preload() {
  handPose = ml5.handPose({ flipped: true });
}

function mousePressed() {
  console.log(hands);
}

function gotHands(results) {
  hands = results;
}

function setup() {
  canvas = createCanvas(640, 480);
  canvas.parent("p5js");
  noLoop();
}

function startVideo() {
  video = createCapture(VIDEO, { flipped: true });
  video.hide();
  handPose.detectStart(video, gotHands);
  loop(); // Restart draw loop to start rendering video frames
}

function draw() {
  image(video, 0, 0);
  if (Pd4Web) {
    Pd4Web.sendFloat("amp", 0);
  }
  if (hands.length > 0) {
    for (let hand of hands) {
      if (hand.confidence > 0.2) {
        let index = hand.index_finger_tip;
        let thumb = hand.thumb_tip;
        noStroke();
        fill(255, 0, 0);
        circle(index.x, index.y, 16);
        circle(thumb.x, thumb.y, 16);
        let d = dist(index.x, index.y, thumb.x, thumb.y) / 480; // amp
        let avg_vertical = 1 - (index.y + thumb.y) / 2 / 480; // freq
        Pd4Web.sendFloat("amp", d);
        Pd4Web.sendFloat("freq", avg_vertical);
      }
    }
  }
}

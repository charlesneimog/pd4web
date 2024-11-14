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
  // Draw the webcam video
  image(video, 0, 0, width, height);

  // Draw all the tracked hand points
  for (let i = 0; i < hands.length; i++) {
    let hand = hands[i];
    for (let j = 0; j < hand.keypoints.length; j++) {
      let keypoint = hand.keypoints[j];
      let whichHand = hand.handedness;
      fill(255, 0, 0);
      noStroke();
      circle(keypoint.x, keypoint.y, 5);
      if (Pd4Web){
        let x = keypoint.x / width;
        let y = keypoint.y / height;
        // check if sendList is available
        if (Pd4Web.sendList){
          Pd4Web.sendList(whichHand[0] + "-" + j, [x, y]);
        } 
      }
    }
  }
}
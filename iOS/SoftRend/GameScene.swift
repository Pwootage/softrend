//
//  GameScene.swift
//  SoftRend
//
//  Created by Pwootage on 12/10/18.
//  Copyright © 2018 Pwootage. All rights reserved.
//

import SpriteKit
import GameplayKit

class GameScene: SKScene {

  private var label: SKLabelNode!
  private var tex: SKMutableTexture!
  private var spriteNode: SKSpriteNode!

  override func didMove(to view: SKView) {
    let path = Bundle.main.path(forResource: "models/teapot", ofType: "obj")

    softrend_startup(path)

    tex = SKMutableTexture(
        size: CGSize(width: Int(softrend_getFBWidth()), height: Int(softrend_getFBHeight())),
//        pixelFormat: Int32(kCVPixelFormatType_128RGBAFloat)
      pixelFormat: Int32(kCVPixelFormatType_32RGBA)
    )

    spriteNode = childNode(withName: "//renderedSprite") as? SKSpriteNode
    spriteNode.texture = tex
    spriteNode.yScale = -1

    // Get label node from scene and store it for use later
    label = childNode(withName: "//helloLabel") as? SKLabelNode

    softrend_render()
    updateTexture()
  }


  func touchDown(atPoint pos: CGPoint) {
  }

  func touchMoved(toPoint pos: CGPoint) {
  }

  func touchUp(atPoint pos: CGPoint) {
  }

  override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
    for t in touches {
      self.touchDown(atPoint: t.location(in: self))
    }
  }

  override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
    for t in touches {
      self.touchMoved(toPoint: t.location(in: self))
    }
  }

  override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
    for t in touches {
      self.touchUp(atPoint: t.location(in: self))
    }
  }

  override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
    for t in touches {
      self.touchUp(atPoint: t.location(in: self))
    }
  }


  override func update(_ currentTime: TimeInterval) {
    softrend_render()
    updateTexture()
    // Called before each frame is rendered
    let f = softrend_getFrame()
    let t = softrend_averageTiming()
    let tFmt = String(format: "%.2f", t)

    // Update UI pos and whatnot
    let smallerDim = min(size.width, size.height)
    spriteNode.size = CGSize(width: smallerDim, height: smallerDim)
    label.position.y = -size.height / 2 + 20
    label.text = "Frame \(f) Avg \(tFmt)ms"
  }

  var i = 0;
  private func updateTexture() {
    let fb = softrend_getFramebuffer()
    let w = Int(softrend_getFBWidth())
    let h = Int(softrend_getFBHeight())

    let count = Int(w * h) // w * h * 4 colors (rgba)
    let fbCopy = UnsafeMutablePointer<Float>.allocate(capacity: count * 4)
    fbCopy.assign(from: fb!.assumingMemoryBound(to: Float.self), count: count * 4)

    tex.modifyPixelData({ (ptr, len) in
      if (len != count * 4) { // len: bytes, count: pixels
          print("BAD BAD LEN DO NOT MATCH \(len) len \(count) count")
          return
        }
      let texPtr = ptr!.assumingMemoryBound(to: UInt8.self)
//      texPtr.assign(from: fbCopy, count: count)
      for y in 0..<h {
        for x in 0..<w {
//          let p = clamp(fbCopy[y * w + x], min: 0, max: 1) * 255
          texPtr[(y * w + x) * 4 + 0] = self.color(fbCopy[(y * w + x) * 4 + 0])//UInt8(p.x)
          texPtr[(y * w + x) * 4 + 2] = self.color(fbCopy[(y * w + x) * 4 + 2])//255//UInt8(p.y)
          texPtr[(y * w + x) * 4 + 1] = self.color(fbCopy[(y * w + x) * 4 + 1])//255//UInt8(p.z)
          texPtr[(y * w + x) * 4 + 3] = UInt8(self.i % 255)//UInt8(p.w)
          self.i+=1
        }
      }
      fbCopy.deallocate()
    })
  }

  func color(_ v: Float) -> UInt8 {
    if (v < 0) {
      return 0
    } else if (v > 1) {
      return 255
    }
    return UInt8(v * 255)
  }

}

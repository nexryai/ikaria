import { describe, it, expect } from 'vitest'
import path from 'path'
import fs from 'fs'
import { getVideoInfoFromPath, resizeImageToWebP } from '../dist/index.js'

/*
describe('getFileInfoFromPath (with actual local file)', () => {
  it('should return valid VideoInfo from a real WebM file', async () => {
    const filePath = path.resolve(__dirname, 'youtube.webm') // テスト用の動画ファイルを同じディレクトリに置く
    const info = await getVideoInfoFromPath(filePath)

    expect(info).toHaveProperty('duration')
    expect(info).toHaveProperty('width')
    expect(info).toHaveProperty('height')
    expect(info).toHaveProperty('videoCodec')
    expect(info).toHaveProperty('audioCodec')
    expect(Array.isArray(info.keyframes)).toBe(true)
  }, 30000) // タイムアウトを30秒に設定
})
*/

// test resize image
describe('resizeImageToWebP (with actual local file)', () => {
  it('should resize image and return buffer', async () => {
    const filePath = path.resolve(__dirname, '1.webp') // テスト用の画像ファイルを同じディレクトリに置く
    const resizedBuffer = await resizeImageToWebP(filePath, 200, 200)

    // write the resized image to a exported.webp
    const outputPath = path.resolve(__dirname, 'exported.webp')

    console.log('Resized image buffer length:', resizedBuffer)

    // save arrayBuffer to file
    fs.writeFileSync(outputPath, Buffer.from(resizedBuffer))

  }, 30000) // タイムアウトを30秒に設定
})

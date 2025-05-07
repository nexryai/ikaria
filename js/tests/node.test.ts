import { describe, it, expect } from 'vitest'
import path from 'path'
import { getVideoInfoFromPath } from '../dist/index.js'

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

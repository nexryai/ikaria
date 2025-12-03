<template>
  <van-nav-bar title="Ikaria Debug UI" :border="false" />
  <van-tabs v-model:active="activeTab" class="tab-header">

    <van-tab title="DASH Conversion" class="tab-contents">
      <div class="file">
        <van-cell-group inset class="form">
          <van-field label="file">
            <template #input>
              <input
                type="file"
                @change="onFileChange"
                accept=".mp4,.mkv,.mp3,.webm"
                :disabled="loading"
              />
            </template>
          </van-field>

          <div v-if="file" style="margin-top: 10px; font-size: 0.9em; color: #666;">
            Selected: {{ file.name }} ({{ (file.size / 1024 / 1024).toFixed(2) }} MB)
          </div>
        </van-cell-group>

        <van-loading color="#000" v-if="loading">
          {{ loadingText }}
        </van-loading>

      </div>
    </van-tab>

    <van-tab title="Trimming" class="tab-contents">
      <div class="file">
        <van-cell-group inset class="form">
          <van-field v-model="trimVideoStartSeconds" label="startSec" placeholder="0.0" />
          <van-field v-model="trimVideoEndSeconds" label="endSec" placeholder="20.0" />

          <van-field label="file">
            <template #input>
              <input
                type="file"
                @change="onTrimmingFileChange"
                accept=".mp4,.mkv,.mp3,.webm"
                :disabled="loading"
              />
            </template>
          </van-field>
        </van-cell-group>

        <van-loading color="#000" v-if="loading">Processing...</van-loading>

        <div v-if="data && data.blobUrl">
          <video :src="data.blobUrl" style="width: 100%" controls></video>
        </div>
      </div>
    </van-tab>
  </van-tabs>
</template>

<script setup lang="ts">
import { ref, watch } from 'vue';
import { showToast } from 'vant';

import { remuxToDash, trimVideo } from '../../../js/src';

const file = ref<File | null>(null);
const data = ref<any>(null);
const activeTab = ref(0);
const loading = ref(false);
const loadingText = ref("Processing...");
const generatedFiles = ref<string[]>([]);

const trimVideoStartSeconds = ref("0");
const trimVideoEndSeconds = ref("20");


async function onFileChange(event: Event) {
  const target = event.target as HTMLInputElement;

  if (!target.files || target.files.length === 0) {
    return;
  }

  const selectedFile = target.files[0];
  file.value = selectedFile; // refへの格納（表示用など）

  console.log("File selected:", selectedFile.name);

  try {
    const opfsRoot = await navigator.storage.getDirectory();

    const fileHandle = await opfsRoot.getFileHandle(selectedFile.name, { create: true });

    const writable = await fileHandle.createWritable();
    await writable.write(selectedFile);
    await writable.close();

    console.log("Written to OPFS via JS");

    remuxToDash(`/opfs/${selectedFile.name}`);

  } catch (err) {
    console.error("Error writing to OPFS:", err);
  }
}

// --- Trimming Tab Handler ---
async function onTrimmingFileChange(event: Event) {
  const target = event.target as HTMLInputElement;
  if (!target.files || target.files.length === 0) return;

  const selected = target.files[0];
  file.value = selected;

  loading.value = true;
  loadingText.value = "Trimming...";

  try {
    const result = await trimVideo(selected, trimVideoStartSeconds.value, trimVideoEndSeconds.value);
    data.value = { blobUrl: result };
  } catch (e) {
    console.error(e);
    showToast('Trimming failed');
  } finally {
    loading.value = false;
  }
}

// タブ切り替え時のリセット
watch(activeTab, (newVal) => {
  if (newVal !== 0) {
    data.value = null;
    file.value = null;
    generatedFiles.value = [];
    // input要素の値をリセットしたければDOM操作が必要ですが、
    // Vueのrefで管理していないネイティブinputなので、
    // タブ切り替えで再レンダリングされればクリアされます。
  }
});
</script>

<style scoped>
.tab-header {
  width: 100%;
  margin: 12px auto;
}

.tab-contents {
  padding: 16px;
  min-height: 100vh;
  background-color: #f6f6f6;
  border-radius: 10px 10px 0 0;
}

.file {
  padding: 16px;
  text-align: center;
}

.form {
  margin-bottom: 24px;
}
</style>

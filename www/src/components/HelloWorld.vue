<template>
  <div class="file">
    <van-form @submit="protocol !== 'file' ? onDownload : null">
      <van-cell-group inset>
        <template v-if="protocol === 'file'">
          <van-uploader
            :after-read="onFile"
            :accept="'.mp4,.mkv,.mp3,.webm'"
          />
        </template>

        <template v-else-if="protocol === 'example'">
          <van-field
            label="サンプル選択"
            is-link
            clickable
            :value="selectedExampleName"
            placeholder="選択してください"
            @click="showExamplePicker = true"
          />
          <van-popup v-model:show="showExamplePicker" position="bottom">
            <van-picker
              :columns="examples.map(e => e.name)"
              @confirm="onExampleConfirm"
              @cancel="showExamplePicker = false"
            />
          </van-popup>
        </template>

        <van-button
          type="primary"
          block
          v-if="protocol !== 'file'"
          native-type="submit"
        >
          ダウンロード
        </van-button>
      </van-cell-group>
    </van-form>

    <van-progress
      v-if="showProgress"
      :percentage="progress"
      color="#1989fa"
      style="margin: 16px"
    />

    <div v-if="data">
      <div v-if="file">
        選択ファイル: {{ file.name }}（{{ file.size }} bytes）
      </div>
      <div v-else>
        URL: {{ url }}（{{ size }} bytes）
      </div>

      <div style="margin-top: 16px">
        <code>{{ JSON.stringify(data) }}</code>
        <video :src="data.blobUrl" style="width: 100%" controls></video>
      </div>

      <div v-if="file && file.type !== 'audio/mpeg'" style="margin-top: 16px">
        <p>todo</p>
      </div>
    </div>
  </div>
</template>

<script setup lang="ts">
import { ref, computed } from 'vue';
import { showToast } from 'vant';

import { getFileInfo } from '../../../js/src';

const protocol = ref('file');
const file = ref<File | null>(null);
const url = ref<string | null>(null);
const size = ref<number | null>(null);
const data = ref<any>(null);
const progress = ref(0);
const showProgress = ref(false);

const showProtocolPicker = ref(false);
const showExamplePicker = ref(false);

const protocolOptions = ['file', 'url', 'example'];

const examples = [
  {
    name: 'Video Counter (10min)',
    value: 'https://video-examples-public.s3.us-west-2.amazonaws.com/video_counter_10min_unfragmented_avc.mp4',
  },
  {
    name: 'Tears of Steel 360p',
    value: 'https://video-examples-public.s3.us-west-2.amazonaws.com/tears-of-steel-360p.mp4',
  }
];

const selectedExampleName = computed(() => {
  const example = examples.find(e => e.value === url.value);
  return example?.name || '';
});

function onExampleConfirm(name: string) {
  const example = examples.find(e => e.name === name);
  if (example) {
    url.value = example.value;
    showExamplePicker.value = false;
  }
}

async function onFile(fileObj: any) {
  console.log("onFile", fileObj);
  const selected = fileObj.file as File;
  file.value = selected;
  await getFileInfo(selected);
  console.log("Done");
}

function onDownload() {
  if (!url.value) {
    showToast('URLを入力してください');
    return;
  }
  showProgress.value = true;
  (window as any).$worker.onmessage = (e: MessageEvent) => {
    data.value = e.data;
  };
  const xhr = new XMLHttpRequest();
  xhr.onprogress = (event) => {
    if (event.lengthComputable) {
      progress.value = Math.floor((event.loaded / event.total) * 100);
    }
  };
  xhr.onload = () => {
    progress.value = 100;
    const downloaded = new File([xhr.response], 'file');
    file.value = downloaded;
    size.value = downloaded.size;
    (window as any).$worker.postMessage(['get_file_info', downloaded]);
    setTimeout(() => (showProgress.value = false), 2000);
  };
  xhr.open('GET', url.value!, true);
  xhr.responseType = 'blob';
  xhr.send();
}
</script>

<style scoped>
.file {
  padding: 16px;
}
</style>

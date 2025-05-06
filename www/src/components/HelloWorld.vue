<template>
  <van-nav-bar title="Ikaria Debug UI" :border=false />
  <van-tabs v-model:active="activeTab" class="tab-header">
    <van-tab title="getVideoInfo" class="tab-contents">
      <div class="file">
        <van-uploader
          :after-read="onFile"
          :accept="'.mp4,.mkv,.mp3,.webm'"
          :upload-icon="'video-o'"
          v-if="!data"
        >
          <template #preview-cover="{ file }">
            <div class="preview-cover van-ellipsis">{{ file.name }}</div>
          </template>
        </van-uploader>

        <div v-if="data">
          <div v-if="file">
            選択ファイル: {{ file.name }}（{{ file.size }} bytes）
          </div>

          <div style="margin-top: 16px">
            <code style="overflow-wrap: anywhere" >{{ JSON.stringify(data) }}</code>
          </div>
        </div>
      </div>
    </van-tab>
    <van-tab title="Trimming" class="tab-contents">
      <div class="file">
        <van-uploader
          :after-read="trimming"
          :accept="'.mp4,.mkv,.mp3,.webm'"
          :upload-icon="'video-o'"
          v-if="!data"
        >
          <template #preview-cover="{ file }">
            <div class="preview-cover van-ellipsis">{{ file.name }}</div>
          </template>
        </van-uploader>

        <div v-if="data">
          <div v-if="file">
            選択ファイル: {{ file.name }}（{{ file.size }} bytes）
          </div>

          <div style="margin-top: 16px">
            <video :src="data.blobUrl" style="width: 100%" controls></video>
          </div>
        </div>
      </div>
    </van-tab>
    <van-tab title="标签 3" class="tab-contents">内容 3</van-tab>
    <van-tab title="标签 4" class="tab-contents">内容 4</van-tab>
  </van-tabs>

</template>

<script setup lang="ts">
import { ref, watch } from 'vue';
import { showToast } from 'vant';

import { getVideoInfo, trimVideo } from '../../../js/src';

const file = ref<File | null>(null);
const data = ref<any>(null);
const activeTab = ref(0);

async function onFile(fileObj: any) {
  console.log("onFile", fileObj);
  const selected = fileObj.file as File;
  file.value = selected;
  showToast({
    message: 'Processing...',
    type: 'loading',
  });
  data.value = await getVideoInfo(selected);
}

async function trimming(fileObj: any) {
  console.log("onFile", fileObj);
  const selected = fileObj.file as File;
  file.value = selected;
  showToast({
    message: 'Processing...',
    type: 'loading',
  });
  data.value = await trimVideo(selected);
}


// タブが切り替えられたらdataをリセット
watch(activeTab, (newVal) => {
  if (newVal !== 0) {
    data.value = null;
    file.value = null;
  }
});
</script>

<style scoped>
.tab-header {
  width: 100%;
  margin: 12px auto 12px auto
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
</style>

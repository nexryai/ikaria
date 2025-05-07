<template>
  <van-nav-bar title="Ikaria Debug UI" :border=false />
  <van-tabs v-model:active="activeTab" class="tab-header">
    <van-tab title="getVideoInfo" class="tab-contents">
      <div class="file">
        <van-cell-group inset class="form">
          <van-field name="uploader" label="file">
            <template #input v-if="!data">
              <van-uploader
                :after-read="onFile"
                :accept="'.mp4,.mkv,.mp3,.webm'"
                :upload-icon="'video-o'"
              >
                <template #preview-cover="{ file }">
                  <div class="preview-cover van-ellipsis">{{ file.name }}</div>
                </template>
              </van-uploader>
            </template>
            <template #input v-else-if="file">
              <div>
                {{ file!.name }}（{{ file!.size }} bytes）
              </div>
            </template>
          </van-field>
        </van-cell-group>

        <van-loading color="#000" v-if="loading">
          Processing...
        </van-loading>

        <div v-if="data">
          <div style="margin-top: 16px">
            <code style="overflow-wrap: anywhere" >{{ JSON.stringify(data) }}</code>
          </div>
        </div>
      </div>
    </van-tab>
    <van-tab title="Trimming" class="tab-contents">
      <div class="file">
        <van-cell-group inset class="form">
          <van-field v-model="trimVideoStartSeconds" label="startSec" placeholder="0.0" />
          <van-field v-model="trimVideoEndSeconds" label="endSec" placeholder="20.0" />
          <van-field name="uploader" label="file">
            <template #input v-if="!data">
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
            </template>
            <template #input v-else-if="file">
              <div>
                {{ file!.name }}（{{ file!.size }} bytes）
              </div>
            </template>
          </van-field>
        </van-cell-group>

        <van-loading color="#000" v-if="loading">
          Processing...
        </van-loading>

        <div v-if="data">
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
const loading = ref(false);

const trimVideoStartSeconds = ref("0");
const trimVideoEndSeconds = ref("20");

async function onFile(fileObj: any) {
  console.log("onFile", fileObj);
  const selected = fileObj.file as File;
  file.value = selected;

  loading.value = true;
  data.value = await getVideoInfo(selected);
  loading.value = false;
}

async function trimming(fileObj: any) {
  console.log("onFile", fileObj);
  const selected = fileObj.file as File;
  file.value = selected;

  loading.value = true;
  data.value =  { blobUrl: await trimVideo(selected, trimVideoStartSeconds.value, trimVideoEndSeconds.value) };
  loading.value = false;
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

.form {
  margin-bottom: 24px;
}
</style>

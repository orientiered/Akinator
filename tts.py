import os
import torch
import torchaudio
import time
import numpy as np
from pydub import AudioSegment
from pydub.playback import play

from scipy.io.wavfile import write

def read_audio(path: str, sampling_rate: int = 24000):
    wav, sr = torchaudio.load(path)
    if wav.size(0) > 1:
        wav = wav.mean(dim=0, keepdim=True)
    if sr != sampling_rate:
        transform = torchaudio.transforms.Resample(
            orig_freq=sr,
            new_freq=sampling_rate
        )
        wav = transform(wav)
        sr = sampling_rate
    assert sr == sampling_rate
    return wav * 0.95

speaker='xenia'
sample_rate = 24000

device = torch.device('cpu')

def init():
    torch.set_num_threads(4)
    denoiseModelFile = 'tts/denoise.pt'
    ttsModelFile = 'tts/v4_ruTTS.pt'

    # if not os.path.isfile(denoiseModelFile):
    #     torch.hub.download_url_to_file('https://models.silero.ai/denoise_models/sns_latest.jit',
    #                                 denoiseModelFile)

    if not os.path.isfile(ttsModelFile):
        torch.hub.download_url_to_file('https://models.silero.ai/models/tts/ru/v4_ru.pt',
                                    ttsModelFile)

    TTS = torch.package.PackageImporter(ttsModelFile).load_pickle("tts_models", "model")
    # DENOISER = torch.jit.load(denoiseModelFile)
    # torch._C._jit_set_profiling_mode(False)
    # torch.set_grad_enabled(False)

    TTS.to(device)
    # DENOISER.to(device)

    exampleText = "Привет"

    print("First inference")
    audio_path = TTS.save_wav(text=exampleText, speaker=speaker, sample_rate=sample_rate)

    # audio = read_audio(audio_path)
    # audio = audio.to(device)
    # denoisedAudio = DENOISER(audio)
    print("Second inference")
    audio_path = TTS.save_wav(text=exampleText, speaker=speaker, sample_rate=sample_rate)

    # audio = read_audio(audio_path)
    # audio = audio.to(device)
    # denoisedAudio = DENOISER(audio)
    print("Initializing completed")

    return TTS

TTS = init()

def getText():
    currentText = []
    while True:
        time.sleep(0.1)
        currentText = open('tts/request.txt', 'r').readlines()
        if len(currentText) >=2:
            if currentText[-1] == '__SPEAK__\n':
                break

    file = open('tts/request.txt', 'w')
    file.close()
    return currentText[:-1]

while (True):
    print("Message to voiceover")
    messages = getText()
    text = ' '.join(messages)
    audio_path = TTS.save_wav(text=text, speaker=speaker, sample_rate=sample_rate)
    print("Sound created")
    audioFile = open(audio_path, 'rb')
    sound = AudioSegment.from_file(audioFile, 'wav', duration = 120)
    play(sound)



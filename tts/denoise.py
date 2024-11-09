import os
import torch
import torchaudio
import numpy as np

from scipy.io.wavfile import write

def read_audio(
    path: str,
    sampling_rate: int = 24000
):
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


device = torch.device('cpu')
torch.set_num_threads(4)
local_file = 'tts/denoise.pt'

if not os.path.isfile(local_file):
    torch.hub.download_url_to_file('https://models.silero.ai/denoise_models/sns_latest.jit',
                                   local_file)

model = torch.jit.load(local_file)
torch._C._jit_set_profiling_mode(False)
torch.set_grad_enabled(False)
model.to(device)

audio_path = 'test.wav'
a = read_audio(audio_path)
a = a.to(device)
out = model(a)
#print('Original: ')
#display(Audio(a.cpu(), rate=24000))
#print('Output: ')
audio_data = out.detach().cpu().numpy()
# scaled = np.int16(audio_data / np.max(np.abs(audio_data)) * 32767)
write('testD.wav', 48000, audio_data)

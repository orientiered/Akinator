import os
import torch

device = torch.device('cpu')
torch.set_num_threads(8)
local_file = 'tts/v4_ruTTS.pt'

if not os.path.isfile(local_file):
    torch.hub.download_url_to_file('https://models.silero.ai/models/tts/ru/v4_ru.pt',
                                   local_file)

model = torch.package.PackageImporter(local_file).load_pickle("tts_models", "model")
model.to(device)

while (True):
    print("Message to voiceover")
    text = input()
    speaker='xenia'
    sample_rate = 24000
    audio_path = model.save_wav(text=text, speaker=speaker, sample_rate=sample_rate)
# print("Hello")
# example_text = '<speak>Меня зовут Кира Йошикагэ. Мне 33 года. Мой дом находится в северо-восточной части Морио, в районе поместий. Работаю в офисе сети магазинов Kame Yu и домой возвращаюсь, самое позднее, в восемь вечера. Не курю, выпиваю изредка. Ложусь спать в 11 вечера и убеждаюсь, что получаю ровно восемь часов сна, несмотря ни на что. Перед сном я пью тёплое молоко, а также минут двадцать уделяю разминке, поэтому до утра сплю без особых проблем. Утром я просыпаюсь, не чувствуя ни усталости, ни стресса, словно младенец. На медосмотре мне сказали, что никаких проблем нет. Я пытаюсь донести, что я обычный человек, который хочет жить спокойной жизнью. Я не забиваю себе голову проблемами вроде побед или поражений, и не обзавожусь врагами, из-за которых не мог бы уснуть. Я знаю наверняка: в таком способе взаимодействия с обществом и кроется счастье. Хотя, если бы мне пришлось сражаться, я бы никому не проиграл.</speak>'
# example_text = '<speak><prosody pitch="x-low">Гоооойда!!!</prosody> братья и сёстры</speak>'
# sample_rate = 24000
# speaker='xenia'
#
# audio_paths = model.save_wav(ssml_text=example_text,
#                              speaker=speaker,
#                              sample_rate=sample_rate)

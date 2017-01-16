

set FLAGS=-map_metadata -1 -flags +bitexact 
set FREQ=4000
ffmpeg -y -i "audio/Hum 4Modified.wav" %FLAGS% -ar %FREQ% -ac 1 -acodec pcm_u8 "audio/idle1.wav"

set FREQ=6400
ffmpeg -y -i "audio/78673__joe93barlow__off0.wav" %FLAGS% -ar %FREQ% -ac 1 -acodec pcm_u8 "audio/off0.wav"
ffmpeg -y -i "audio/SaberOnTrimmed.wav" %FLAGS% -ar %FREQ% -ac 1 -acodec pcm_u8 "audio/on0.wav"
ffmpeg -y -i "audio/78676__joe93barlow__strike1.wav" %FLAGS% -ar %FREQ% -ac 1 -acodec pcm_u8 "audio/strike1.wav"
ffmpeg -y -i "audio/78677__joe93barlow__strike2.wav" %FLAGS% -ar %FREQ% -ac 1 -acodec pcm_u8 "audio/strike2.wav"
ffmpeg -y -i "audio/78675__joe93barlow__strike0.wav" %FLAGS% -ar %FREQ% -ac 1 -acodec pcm_u8 "audio/strike0.wav"

wav2c "audio/idle1.wav" "src/Lightsaber/idle1.h" idle1
wav2c "audio/off0.wav" "src/Lightsaber/off0.h" off0
wav2c "audio/on0.wav" "src/Lightsaber/on0.h" on0
wav2c "audio/strike1.wav" "src/Lightsaber/strike1.h" strike1
wav2c "audio/strike2.wav" "src/Lightsaber/strike2.h" strike2
wav2c "audio/strike0.wav" "src/Lightsaber/strike0.h" strike0


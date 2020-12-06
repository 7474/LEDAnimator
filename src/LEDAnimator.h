#ifndef __LEDANIMATOR_H__
#define __LEDANIMATOR_H__

#include <Adafruit_NeoPixel.h>

struct LEDAnimatorCRGB
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

enum LEDAnimatorFrameType
{
    Solid = 0,
    Fade = 1,
    AtoTume = 2,
    MaeTume = 3
};

// https://arduino.stackexchange.com/questions/69174/is-stdarray-from-the-c-stl-safe-to-use-on-arduino-does-it-use-dynamic-mem
template <size_t LEDAnimator_NUM_LEDS>
struct LEDAnimatorFrame
{
    LEDAnimatorFrameType frameType;
    uint16_t millis;
    LEDAnimatorCRGB leds[LEDAnimator_NUM_LEDS];
};

template <size_t LEDAnimator_NUM_LEDS>
class LEDAnimator
{
public:
    LEDAnimator(uint16_t n, uint16_t pin = 6, neoPixelType type = NEO_GRB + NEO_KHZ800);
    ~LEDAnimator();

    bool loadBySD(String &path);
    void setFrames(LEDAnimatorFrame<LEDAnimator_NUM_LEDS> *frames, size_t frameLength);

    void start();
    void stop();
    bool isPlaying();

    void update();

    void printFrames();
    void printFrame(LEDAnimatorFrame<LEDAnimator_NUM_LEDS> *frame);

private:
    uint16_t ledNum;
    bool loadedBySD;
    size_t frameLength;
    LEDAnimatorFrame<LEDAnimator_NUM_LEDS> *frames;

    Adafruit_NeoPixel pixels;
    LEDAnimatorCRGB pixelsBuf[LEDAnimator_NUM_LEDS];

    bool loop;
    bool playing;
    unsigned long lastMillis;
    unsigned long lastUpdateMillis;
    unsigned long currentFrameStartMillis;
    size_t currentFrameNum;
    LEDAnimatorFrame<LEDAnimator_NUM_LEDS> *currentFrame;
    LEDAnimatorFrame<LEDAnimator_NUM_LEDS> *lastFrame;

    void fillSolid(LEDAnimatorFrame<LEDAnimator_NUM_LEDS> *frame);
    void updatePixels();
};

template <size_t LEDAnimator_NUM_LEDS>
LEDAnimator<LEDAnimator_NUM_LEDS>::LEDAnimator(uint16_t n, uint16_t pin, neoPixelType type)
    : pixels(n, pin, type)
{
    this->ledNum = n;

    // this->loop = false;
    this->loop = true;
    this->playing = false;
}

template <size_t LEDAnimator_NUM_LEDS>
LEDAnimator<LEDAnimator_NUM_LEDS>::~LEDAnimator()
{
}

template <size_t LEDAnimator_NUM_LEDS>
bool LEDAnimator<LEDAnimator_NUM_LEDS>::loadBySD(String &path)
{
    // TODO
    return false;
}

template <size_t LEDAnimator_NUM_LEDS>
void LEDAnimator<LEDAnimator_NUM_LEDS>::setFrames(LEDAnimatorFrame<LEDAnimator_NUM_LEDS> *frames, size_t frameLength)
{
    this->playing = false;
    this->frames = frames;
    this->frameLength = frameLength;
}

template <size_t LEDAnimator_NUM_LEDS>
void LEDAnimator<LEDAnimator_NUM_LEDS>::start()
{
    this->playing = false;

    this->lastMillis = 0;
    this->lastUpdateMillis = 0;
    this->currentFrameStartMillis = 0;
    this->currentFrameNum = 0;
    this->currentFrame = this->frames;
    this->lastFrame = NULL;

    this->playing = true;
}

template <size_t LEDAnimator_NUM_LEDS>
void LEDAnimator<LEDAnimator_NUM_LEDS>::stop()
{
    this->playing = false;
}

template <size_t LEDAnimator_NUM_LEDS>
bool LEDAnimator<LEDAnimator_NUM_LEDS>::isPlaying()
{
    return this->playing;
}

template <size_t LEDAnimator_NUM_LEDS>
void LEDAnimator<LEDAnimator_NUM_LEDS>::update()
{
    // Serial.println("LEDAnimator::update");
    if (!this->playing)
    {
        return;
    }

    bool updateFrame = false;
    bool updatePixels = false;
    unsigned long nowMillis = millis();
    unsigned long inFrameMillis;
    if (this->lastMillis == 0)
    {
        this->lastMillis = nowMillis;
        this->currentFrameStartMillis = nowMillis;
        inFrameMillis = 0;
        updateFrame = true;
        updatePixels = true;
    }
    else
    {
        inFrameMillis = nowMillis - this->currentFrameStartMillis;
    }

    // Serial.println(nowMillis);
    // Serial.println(this->currentFrameNum);
    // Serial.println(inFrameMillis);
    while (this->currentFrameNum < this->frameLength && this->currentFrame->millis < inFrameMillis)
    {
        updateFrame = true;
        updatePixels = true;
        // Serial.println(nowMillis);
        // Serial.println(inFrameMillis);
        // Serial.println("to next frame");
        // Serial.println(this->currentFrameNum);
        inFrameMillis -= this->currentFrame->millis;
        this->currentFrameStartMillis += this->currentFrame->millis;

        this->lastFrame = this->currentFrame;
        this->currentFrame++;
        this->currentFrameNum++;
        if (this->currentFrameNum >= this->frameLength)
        {
            if (this->loop)
            {
                this->start();
            }
            else
            {
                this->stop();
            }
            return;
        }
    }

    switch (this->currentFrame->frameType)
    {
    case Solid:
        if (updateFrame)
        {
            this->fillSolid(this->currentFrame);
            // Serial.println("fillSolid");
            // this->printFrame(this->currentFrame);
        }
        break;
    default:
        // TODO impliment
        break;
    }
    // TODO 次の描画フレーム向けのバッファ構築と転送までやるのと、転送をやるのに分ける。今は出来高。
    // about 30 microseconds per RGB pixel, 40 for RGBW pixels
    if (updatePixels)
    {
        this->updatePixels();
        this->lastUpdateMillis = millis();
    }

    this->lastMillis = millis();
}

template <size_t LEDAnimator_NUM_LEDS>
void LEDAnimator<LEDAnimator_NUM_LEDS>::printFrames()
{
    Serial.println("printFrames");
    Serial.println("[");
    for (int i = 0; i < this->frameLength; i++)
    {
        this->printFrame(&this->frames[i]);
        if (i + 1 != this->frameLength)
        {
            Serial.println(",");
        }
    }
    Serial.println("]");
}

template <size_t LEDAnimator_NUM_LEDS>
void LEDAnimator<LEDAnimator_NUM_LEDS>::printFrame(LEDAnimatorFrame<LEDAnimator_NUM_LEDS> *frame)
{
    Serial.print("[");
    Serial.print(frame->frameType);
    Serial.print(",");
    Serial.print(frame->millis);
    Serial.print(",[");
    for (int j = 0; j < this->ledNum; j++)
    {
        Serial.print("[");
        Serial.print(frame->leds[j].r);
        Serial.print(",");
        Serial.print(frame->leds[j].g);
        Serial.print(",");
        Serial.print(frame->leds[j].b);
        Serial.print("]");
    }
    Serial.print("]]");
}

template <size_t LEDAnimator_NUM_LEDS>
void LEDAnimator<LEDAnimator_NUM_LEDS>::fillSolid(LEDAnimatorFrame<LEDAnimator_NUM_LEDS> *frame)
{
    // TODO 輝度調整
    for (int i = 0; i < this->ledNum; i++)
    {
        this->pixelsBuf[i] = frame->leds[i];
    }
    // XXX こっちだけでもいいかも
    for (int i = 0; i < this->ledNum; i++)
    {
        this->pixels.setPixelColor(i, this->pixels.Color(
                                          this->pixelsBuf[i].r,
                                          this->pixelsBuf[i].g,
                                          this->pixelsBuf[i].b));
    }
}

template <size_t LEDAnimator_NUM_LEDS>
void LEDAnimator<LEDAnimator_NUM_LEDS>::updatePixels()
{
    pixels.show();
}

#endif // __LEDANIMATOR_H__
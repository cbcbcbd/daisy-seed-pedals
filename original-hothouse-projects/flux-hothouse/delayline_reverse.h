// Reverse Delay Line
// Based on Neptune implementation by Adam Fulford
// Adapted for FLUX project

#pragma once
#ifndef DELAYLINE_REVERSE_H
#define DELAYLINE_REVERSE_H

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

template <typename T, size_t max_size>
class DelayLineReverse
{
  public:
    DelayLineReverse() {}
    ~DelayLineReverse() {}
    
    /** Initialize the delay line */
    void Init() 
    { 
        Reset(); 
    }
    
    /** Clear buffer and reset pointers */
    void Reset()
    {
        delay_ = 2400;      // Min reverse delay time
        fadetime_ = 2300;   // Crossfade time in samples
        
        for(size_t i = 0; i < max_size; i++)
        {
            line_[i] = T(0);
        }
        
        write_ptr_ = 0;
        read_ptr1_ = 0;
        read_ptr2_ = 0;
        head_diff_ = 0;
        playing_head_ = false;
        fade_pos_ = 0.0f;
        fading_ = false;
    }

    /** Set delay time in samples */
    inline void SetDelay(size_t delay)
    {
        frac_ = 0.0f;
        delay_ = delay < max_size ? delay : max_size - 1;
    }

    /** Set delay time with fractional component */
    inline void SetDelay(float delay)
    {
        int32_t int_delay = static_cast<int32_t>(delay);
        frac_ = delay - static_cast<float>(int_delay);
        delay_ = static_cast<size_t>(int_delay) < max_size ? int_delay : max_size - 1;
    }

    /** Write sample to delay line */
    inline void Write(const T sample)
    {
        line_[write_ptr_] = sample;
        
        // Advance write pointer forward
        write_ptr_ = (write_ptr_ + 1) % max_size;
        
        // Increment head difference
        head_diff_ = (head_diff_ + 1) % delay_;
        
        // Advance read pointers backward
        read_ptr1_ = (read_ptr1_ - 1 + max_size) % max_size;
        read_ptr2_ = (read_ptr2_ - 1 + max_size) % max_size;
        
        // Check if we need to start crossfading
        if (head_diff_ > (delay_ - fadetime_ - 1))
        {
            if(!fading_)
            {
                fading_ = true;
                
                if(!playing_head_) 
                {
                    // Jump ptr2 to position near write pointer
                    read_ptr2_ = (write_ptr_ - 1 + max_size) % max_size;
                }
                else
                {
                    // Jump ptr1 to position near write pointer
                    read_ptr1_ = (write_ptr_ - 1 + max_size) % max_size;
                }
            }
        }
        
        // Update crossfade position
        if(fading_)
        {
            if(!playing_head_)
            {
                // Fade from head1 to head2
                fade_pos_ = fade_pos_ + (1.0f / fadetime_);
                if (fade_pos_ >= 1.0f)
                {
                    fade_pos_ = 1.0f;
                    fading_ = false;
                    playing_head_ = true;
                }
            }
            else
            {
                // Fade from head2 to head1
                fade_pos_ = fade_pos_ - (1.0f / fadetime_);
                if (fade_pos_ <= 0.0f)
                {
                    fade_pos_ = 0.0f;
                    fading_ = false;
                    playing_head_ = false;
                }
            }
        }
    }

    /** Read in reverse direction with crossfading */
    inline const T Read() const
    {
        T a1 = line_[read_ptr1_];
        T a2 = line_[read_ptr2_];
        
        // Use Hann window for crossfading
        float scalar_1 = sinf(fade_pos_ * (M_PI * 0.5f));
        float scalar_2 = sinf((1.0f - fade_pos_) * (M_PI * 0.5f));
        
        return (a2 * scalar_1) + (a1 * scalar_2);
    }

  private:
    float  frac_;
    size_t write_ptr_;
    size_t read_ptr1_;
    size_t read_ptr2_;
    size_t delay_;
    size_t head_diff_;
    T      line_[max_size];
    size_t fadetime_;
    bool   playing_head_;
    float  fade_pos_;
    bool   fading_;
};

#endif

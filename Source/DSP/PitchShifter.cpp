/*
  ==============================================================================

    PitchShifter.cpp
    Created: 6 Nov 2024 11:52:44am
    Author:  Jhonatan

  ==============================================================================
*/
#include "PitchShifter.h"

PitchShift::PitchShift() {}
PitchShift::~PitchShift() {}

// Prepara la clase con la frecuencia de muestreo
// Prepares the class with the sample rate
void PitchShift::prepare(double theSampleRate)
{
    sampleRate = static_cast<float>(theSampleRate);  // Asigna la frecuencia de muestreo
    writePosition = 0;  // Inicializa la posición de escritura del buffer
    std::fill(std::begin(delayBuffer), std::end(delayBuffer), 0.0f); // buffer to 0
}

// Establece la cantidad de semitonos para el cambio de tono
// Sets the number of semitones for the pitch shift
void PitchShift::setSemitones(int semitones)
{
    float tr = std::pow(2.0f, static_cast<float>(semitones) / 12.0f);  // Calcula el factor de transposición (2^(semitones/12))
    delayRate = 1.0f - tr;  // Calcula la tasa de cambio del retardo

    // Si el cambio de tono es negativo, el retardo inicial es cero, de lo contrario calculamos el retardo
    // If the pitch shift is negative, the initial delay is set to zero, otherwise calculate the delay
    if (delayRate > 0)
    {
        delay = 0.0f;
    }
    else
    {
        delay = static_cast<float>(bufferSize) * -delayRate;
    }
}

// Procesa el buffer de audio y aplica el efecto de cambio de tono
// Processes the audio buffer and applies the pitch shift effect
void PitchShift::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples(); 
    const int numChannels = buffer.getNumChannels();  

    // Bucle que recorre todas las muestras
    // Loop that goes through all the samples
    for (int i = 0; i < numSamples; ++i)
    {
        // Bucle que recorre todos los canales (por ejemplo, para estéreo se procesan dos canales)
        // Loop that goes through all channels (for stereo, two channels are processed)
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);

            // Calcula el retardo entero y fraccionado
            // Calculates the integer and fractional part of the delay
            int intDelay = static_cast<int>(floorf(delay));
            float frac = delay - intDelay;

            // Calcula la posición de lectura del buffer de delay
            // Calculates the read position in the delay buffer
            int readPosition = writePosition - intDelay;
            if (readPosition < 0) 
                readPosition += bufferSize;

            // Calcula el índice de la siguiente muestra en el buffer de delay
            // Calculates the next sample index in the delay buffer
            int nextSampleIndex = (readPosition + 1) % bufferSize;

            // Realiza la interpolación lineal entre la muestra actual y la siguiente en el buffer de delay
            // Performs linear interpolation between the current and next sample in the delay buffer
            float delayedSample = (1.0f - frac) * delayBuffer[readPosition] + frac * delayBuffer[nextSampleIndex];

            // Guarda la muestra actual en el buffer de delay
            // Stores the current sample in the delay buffer
            delayBuffer[writePosition] = channelData[i];

            // Sustituye la muestra actual por la muestra retardada
            // Replaces the current sample with the delayed sample
            channelData[i] = delayedSample;
        }

        // Avanza la posición de escritura en el buffer circular
        // Advances the write position in the circular buffer
        writePosition = (writePosition + 1) % bufferSize;

        // Ajusta el retardo para la siguiente muestra
        // Adjusts the delay for the next sample
        delay += delayRate;
    }
}

// Alternativa: Proceso de pitch shifting separado por canal
// Alternative: Channel-separated pitch shifting process
/*
void PitchShift::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();  // Gets the number of samples in the buffer
    const int numChannels = buffer.getNumChannels();  // Gets the number of channels (e.g., stereo)

    float oldDelay = delay;  // Guarda el valor original del retardo

    // Recorre todos los canales
    // Loops through all channels
    for (int channel = 0; channel < numChannels; ++channel)
    {
        delay = oldDelay;  // Resets the delay for the current channel

        // Procesa las muestras del canal actual
        // Processes the samples for the current channel
        for (int i = 0; i < numSamples; ++i)
        {
            float* channelData = buffer.getWritePointer(channel);  // Gets the write pointer for the channel

            // Calcula el retardo entero y fraccionado
            // Calculates the integer and fractional delay
            int intDelay = static_cast<int>(floorf(delay));
            float frac = delay - intDelay;  // Fractional part of the delay

            // Calcula la posición de lectura en el buffer de delay
            // Calculates the read position in the delay buffer
            int readPosition = writePosition - intDelay;
            if (readPosition < 0)  // Adjust the read position if it's negative
                readPosition += bufferSize;

            // Cálculo de la siguiente muestra en el buffer de delay
            // Calculates the next sample in the delay buffer
            int nextSampleIndex = (readPosition + 1) % bufferSize;
            float delayedSample = (1.0f - frac) * delayBuffer[readPosition] + frac * delayBuffer[nextSampleIndex];

            // Guarda la muestra actual en el buffer de delay
            // Stores the current sample in the delay buffer
            delayBuffer[writePosition] = channelData[i];

            // Sustituye la muestra actual con la muestra retardada
            // Replaces the current sample with the delayed sample
            channelData[i] = delayedSample;

            // Avanza la posición de escritura en el buffer circular
            // Advances the write position in the circular buffer
            writePosition = (writePosition + 1) % bufferSize;

            // Ajusta el retardo para la siguiente muestra
            // Adjusts the delay for the next sample
            delay += delayRate;
        }
    }
}
*/

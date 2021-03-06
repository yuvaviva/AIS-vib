/*
Copyright(c) 2021-2022 jvde.github@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <cstring>

#include "HACKRF.h"

namespace Device {

	//---------------------------------------
	// Device HACKRF

#ifdef HASHACKRF

	void HACKRF::Open(uint64_t h)
	{
		if(!list) throw "HACKRF: cannot open device, internal error.";
		if (h > list->devicecount) throw "HACKRF: cannot open device.";

		int result = hackrf_open_by_serial(list->serial_numbers[h],&device);
		if (result != HACKRF_SUCCESS) throw "HACKRF: cannot open device.";

		setSampleRate(6000000);

		Device::Open(h);
	}

	void HACKRF::Close()
	{
		Device::Close();
		hackrf_close(device);
	}

	void HACKRF::Play()
	{
		applySettings();

		if (hackrf_start_rx(device, HACKRF::callback_static, this) != HACKRF_SUCCESS) throw "HACKRF: Cannot open device";
		Device::Play();

		SleepSystem(10);
	}

	void HACKRF::Stop()
	{
		if (Device::isStreaming())
		{
			Device::Stop();
			hackrf_stop_rx(device);
		}
	}

	int HACKRF::callback_static(hackrf_transfer* tf)
	{
		((HACKRF*)tf->rx_ctx)->callback(tf->buffer, tf->valid_length);
		return 0;
	}

	void HACKRF::callback(uint8_t* data, int len)
	{
		RAW r = { Format::CS8, data, len };
		Send(&r, 1);
	}

	void HACKRF::applySettings()
	{
		if (hackrf_set_amp_enable(device, preamp ? 1 : 0) != HACKRF_SUCCESS) throw "HACKRF: cannot set amp.";
		if (hackrf_set_lna_gain(device, LNA_Gain) != HACKRF_SUCCESS) throw "HACKRF: cannot set LNA gain.";
		if (hackrf_set_vga_gain(device, VGA_Gain) != HACKRF_SUCCESS) throw "HACKRF: cannot set VGA gain.";

		if (hackrf_set_sample_rate(device, sample_rate) != HACKRF_SUCCESS) throw "HACKRF: cannot set sample rate.";
		if (hackrf_set_baseband_filter_bandwidth(device, hackrf_compute_baseband_filter_bw(sample_rate)) != HACKRF_SUCCESS) throw "HACKRF: cannot set bandwidth filter to auto.";
		if (hackrf_set_freq(device, frequency) != HACKRF_SUCCESS) throw "HACKRF: cannot set frequency.";
	}

	void HACKRF::getDeviceList(std::vector<Description>& DeviceList)
	{
		list = hackrf_device_list();

		for (int i = 0; i < list->devicecount; i++)
		{
			if (list->serial_numbers[i])
			{
				std::stringstream serial;
				serial << std::uppercase << list->serial_numbers[i];
				DeviceList.push_back(Description("HACKRF", "HACKRF", serial.str(), (uint64_t)i, Type::HACKRF));
			}
		}
	}

	void HACKRF::Print()
	{
		std::cerr << "Hackrf Settings: -gf";
		std::cerr << " preamp ";
		if (preamp) std::cerr << "ON"; else std::cerr << "OFF";
		std::cerr << " lna " << LNA_Gain;
		std::cerr << " vga " << VGA_Gain;
		std::cerr << std::endl;
	}

	void HACKRF::Set(std::string option, std::string arg)
	{
		Util::Convert::toUpper(option);
		Util::Convert::toUpper(arg);

		if (option == "LNA")
		{
			LNA_Gain = ((Util::Parse::Integer(arg, 0, 40) + 4) / 8) * 8;
		}
		else if (option == "VGA")
		{
			VGA_Gain = ((Util::Parse::Integer(arg, 0, 62) + 1) / 2) * 2;
		}
		else if (option == "PREAMP")
		{
			preamp = Util::Parse::Switch(arg);
		}
		else
			throw "Invalid setting for HACKRF.";
	}
#endif
}

#pragma once
#include "RpConnection.h"

#include "RpGui.h"
#include <Engine/Engine.h>
#include <Base/Math/Complex.h>
#include <Base/Datastructures/Stream.h>
#include "Filter.h"

namespace PH::RpGui {

	struct TransferFunction {
		Engine::String name;
		Engine::String currentcommand;
		RpConnection connection;

		Engine::ArrayList<Filter> filters;
		bool32 lowprecision = false;
		uint32 decimation = 1;
		bool32 b_invert = false;

		const real64 getFilterSampleRate() const { return (real64)RP_FPGA_SAMPLERATE / decimation; };
	};

	inline real64 prewarp(real64 f_continuous, real64 f_sample) {
		return 2 * f_sample * atan(f_continuous / (2 * f_sample));
	}

	inline void bilinearTransform(real64 acoefs[], real64 dcoefs[], real64 f_sample)
	{
		double b0, b1, b2, a0, a1, a2;
		double bz0, bz1, bz2, az0, az1, az2;

		b0 = acoefs[0]; b1 = acoefs[1]; b2 = acoefs[2];
		a0 = acoefs[3]; a1 = acoefs[4]; a2 = acoefs[5];

		real64 T = 1 / f_sample;
		real64 K = 2 / T;
		real64 Ks = K * K;

		az0 = a0 * Ks + a1 * K + a2;

		bz0 = ((b0 * Ks) + (b1 * K) + b2) / az0;
		bz1 = (2 * b2 - (2 * b0 * Ks)) / az0;
		bz2 = ((b0 * Ks) - (b1 * K) + b2) / az0;

		az1 = ((2 * a2) - (2 * a0 * Ks)) / az0;
		az2 = ((a0 * Ks) - (a1 * K) + a2) / az0;

		az0 = 1.0;

		dcoefs[0] = bz0; dcoefs[1] = bz1; dcoefs[2] = bz2;
		dcoefs[3] = az0; dcoefs[4] = az1; dcoefs[5] = az2;
	}

	inline PH::int16 convertToFixedPoint(real64 in) {
		uint64 mult = 1 << 14;
		real64 result = in * mult;
		return (PH::int16)(result > 0.0f ? result + 0.5 : result - 0.5);
	}

	//should maybe floor or round the value correctly but I can do that another time
	inline PH::int32 convertToFixedPoint32(real64 in) {
		uint64 mult = 1 << 30;
		real64 result = in * mult;
		return (PH::int32)(result > 0.0f ? result + 0.5 : result - 0.5);
	}

	inline Base::Complex<real64> applyFilter(Base::Complex<real64> s, const BiQuadCoefficients& coeffs) {
		return (coeffs.b[0] * s * s + coeffs.b[1] * s + coeffs.b[2]) / (coeffs.a[0] * s * s + coeffs.a[1] * s + coeffs.a[2]);
	}

	inline BiQuadCoefficients bilinearTransform(BiQuadCoefficients continuous, real64 targetfs) {
		BiQuadCoefficients result;
		bilinearTransform(continuous.b, result.b, targetfs);
		return result;
	}

	inline char valToHex(uint16 val) {
		if (val >= 0 && val < 10) {
			return val + '0';
		}

		if (val >= 10 && val < 16) {
			return val + 'A' - 10;
		}

		return '0';
	}

	inline void writeHexVal32(uint32 coeff, char* buffer) {
		//clear the buffer
		for (uint32 i = 0; i < 9; i++) {
			buffer[i] = '\0';
		}
		buffer[7] = valToHex(coeff & 0x0000000F);
		buffer[6] = valToHex((coeff >> 4) & 0x0000000F);
		buffer[5] = valToHex((coeff >> 8) & 0x0000000F);
		buffer[4] = valToHex((coeff >> 12) & 0x0000000F);
		buffer[3] = valToHex((coeff >> 16) & 0x0000000F);
		buffer[2] = valToHex((coeff >> 20) & 0x0000000F);
		buffer[1] = valToHex((coeff >> 24) & 0x0000000F);
		buffer[0] = valToHex((coeff >> 28) & 0x0000000F);
	}

	inline void writeHexVal(uint16 coeff, char* buffer) {

		//clear the buffer
		for (uint32 i = 0; i < 5; i++) {
			buffer[i] = '\0';
		}

		buffer[3] = valToHex(coeff & 0x000F);
		buffer[2] = valToHex((coeff >> 4) & 0x000F);
		buffer[1] = valToHex((coeff >> 8) & 0x000F);
		buffer[0] = valToHex((coeff >> 12) & 0x000F);
	}

	inline Engine::String generateRpFilterString32(const BiQuadCoefficients& dcoeffs) {
		Base::Stream<Engine::Allocator> s = Base::Stream<Engine::Allocator>::create(100);

		int32 a1 = convertToFixedPoint32(dcoeffs.a[1]);
		int32 a2 = convertToFixedPoint32(dcoeffs.a[2]);

		int32 b0 = convertToFixedPoint32(dcoeffs.b[0]);
		int32 b1 = convertToFixedPoint32(dcoeffs.b[1]);
		int32 b2 = convertToFixedPoint32(dcoeffs.b[2]);

		s << "export PATH=$PATH:/opt/redpitaya/bin;";

		//first coeff at 0x41200000
		char buffer[9];
		s << " monitor 0x41200000 0x";
		writeHexVal32(b0, buffer);
		s << buffer;

		s << "; monitor 0x41200008 0x";
		writeHexVal32(b1, buffer);
		s << buffer;

		s << "; monitor 0x41210000 0x";
		writeHexVal32(b2, buffer);
		s << buffer;

		s << "; monitor 0x41210008 0x";
		writeHexVal32(a1, buffer);
		s << buffer;

		s << "; monitor 0x41220000 0x";
		writeHexVal32(a2, buffer);
		s << buffer;

		Engine::String result = s.createString<Engine::Allocator>();
		Base::Stream<Engine::Allocator>::destroy(&s);

		return result;
	}

	inline Engine::String generateRPfilterString(const BiQuadCoefficients& dcoeffs) {

		Base::Stream<Engine::Allocator> s = Base::Stream<Engine::Allocator>::create(100);
		

		int16 a1 = convertToFixedPoint(dcoeffs.a[1]);
		int16 a2 = convertToFixedPoint(dcoeffs.a[2]);

		int16 b0 = convertToFixedPoint(dcoeffs.b[0]);
		int16 b1 = convertToFixedPoint(dcoeffs.b[1]);
		int16 b2 = convertToFixedPoint(dcoeffs.b[2]);

		s << "export PATH=$PATH:/opt/redpitaya/bin;";

		//first 2 coeffs at 0x41200000
		s << "monitor 0x41200000 0x";
		char buffer[5];
		writeHexVal((uint16)a1, buffer);
		s << buffer;

		writeHexVal((uint16)a2, buffer);
		s << buffer;

		//next 2 coeffs at 0x41200008
		s << "; monitor 0x41200008 0x";
		writeHexVal((uint16)b0, buffer);
		s << buffer;

		writeHexVal((uint16)b1, buffer);
		s << buffer;
		
		//last coeff at 0x41210000
		s << "; monitor 0x41210000 0x";
		writeHexVal((uint16)b2, buffer);
		s << buffer;
		s << "0000";

		Engine::String result = s.createString<Engine::Allocator>();
		Base::Stream<Engine::Allocator>::destroy(&s);
		return result;
	}

	inline void recalculateFilter(Filter& filter) {
		filter.calculateCoefficients();
		filter.getBiquadCoefficients();
	}

	inline void serializeTransferFunction(const TransferFunction& function, YAML::Emitter& out) {

		out << YAML::BeginMap;
		out << YAML::Key << "name" << YAML::Value << function.name.getC_Str();
		out << YAML::Key << "remote" << YAML::Value << function.connection.remoteip.getC_Str();
		out << YAML::Key << "decimation" << YAML::Value << function.decimation;
		out << YAML::Key << "lowprecision" << YAML::Value << function.lowprecision;
		out << YAML::Key << "filters" << YAML::Value << YAML::BeginSeq;

		for (const auto& filter : function.filters) {
			filter.serialize(out);
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;
	}

	inline TransferFunction deserializeTransferFunction(const YAML::Node& t) {
		TransferFunction result;
		result.currentcommand = Engine::String::create("");
		result.name = t["name"].as<Engine::String>();
		result.connection.remoteip = t["remote"].as<Engine::String>();
		result.filters = Engine::ArrayList<Filter>::create(1);

		if (t["lowprecision"]) {
			result.lowprecision = t["lowprecision"].as<bool32>();
		}
		else {
			result.lowprecision = false;
		}

		if (t["decimation"]) {
			result.decimation = (real64)t["decimation"].as<uint32>();
		}
		else {
			result.decimation = RpGui::standard_decimation;
		}

		for (const auto& f : t["filters"]) {
			result.filters.pushBack(Filter::deserialize(f));
		}

		return result;
	}


	inline void sentFilterToRp(Filter& f, real64 targetfs, RpGui::RpConnection* connection, bool32 lowprecision) {

		const real64 unwarpedCF = f.cutoffFrequency;
		const real64 unwarpedACF = f.antiCutoffFrequency;

		f.cutoffFrequency = prewarp(2 * M_PI * unwarpedCF, targetfs);
		f.antiCutoffFrequency = prewarp(2 * M_PI * unwarpedACF, targetfs);

		BiQuadCoefficients dcoeffs = bilinearTransform(f.getBiquadCoefficients(), targetfs / (2 * M_PI));

		f.cutoffFrequency = unwarpedCF;
		f.antiCutoffFrequency = unwarpedACF;

		Engine::String rpcommand;

		if (lowprecision) {
			rpcommand = generateRPfilterString(dcoeffs);
		}
		else {
			rpcommand = generateRpFilterString32(dcoeffs);
		}

		if (connection->open) {
			connection->commandqueue.push({ rpcommand });
			connection->commandqueue.push({ Engine::String::create("export PATH=$PATH:/opt/redpitaya/bin; monitor 0x41230000 1; monitor 0x41230000 0") });
			//connection->commandqueue.push({ Engine::String::create("monitor 0x41230000 0") });
			ReleaseSemaphore(connection->semaphore, 1, nullptr);
		}
	}
}

#include "TransferFunction.h"

#ifdef _DEBUG
	#undef _DEBUG
	#include <Python.h>
	#define _DEBUG
	#else
	#include <Python.h>
#endif

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>

#include "Context.h"
#include <Engine/Engine.h>


namespace py = pybind11;

using namespace PH;
//using namespace PH::RpGui;

// actually usable functions in python!
void setFilterCutoff(int filternumber, float cutoff) {
	RpGui::context->activetransferfunctions[0].filters[filternumber].cutoff = cutoff;

	recalculateFilter(&RpGui::context->activetransferfunctions[0].filters[filternumber]);
	sentFilterToRp(RpGui::context->activetransferfunctions[0].filters[filternumber], RpGui::targetfs, &RpGui::context->activetransferfunctions[0].connection);
}

//actually usable functions in python!
void setFilterQfactor(int filternumber, float qfactor) {
	RpGui::context->activetransferfunctions[0].filters[filternumber].Qfactor = qfactor;

	recalculateFilter(&RpGui::context->activetransferfunctions[0].filters[filternumber]);
	sentFilterToRp(RpGui::context->activetransferfunctions[0].filters[filternumber], RpGui::targetfs, &RpGui::context->activetransferfunctions[0].connection);
}

//actually usable functions in python!
void addPlot(py::array_t<float> freq, py::array_t<float> magnitude, py::array_t<float> phase, char* name, glm::vec4 color = glm::vec4(1.0f)) {

	auto freqbuf = freq.request();
	float* freqptr = static_cast<float*>(freqbuf.ptr);
	size_t freqsize = freqbuf.size;

	auto magbuf = magnitude.request();
	float* magptr = static_cast<float*>(magbuf.ptr);
	size_t magsize = magbuf.size;

	auto phasebuf = phase.request();
	float* phaseptr = static_cast<float*>(phasebuf.ptr);
	size_t phasesize = phasebuf.size;

	PH_DEBUG_ASSERT(freqsize == magsize && freqsize == phasesize, "Frequency, magnitude and phase arrays must have the same size!");

	Engine::ArrayList<glm::vec2> magpoints = Engine::ArrayList<glm::vec2>::create(freqsize);
	for (uint32 i = 0; i < freqsize; i++) {
		magpoints.pushBack({ freqptr[i], magptr[i] });
	}

	Engine::ArrayList<glm::vec2> phasepoints = Engine::ArrayList<glm::vec2>::create(freqsize);
	for (uint32 i = 0; i < freqsize; i++) {
		phasepoints.pushBack({ freqptr[i], phaseptr[i] });
	}

	RpGui::INFO << "Adding plot with name: " << name << " and " << freqsize << " points\n";

	RpGui::PlotData plotdata{};
	plotdata.name = Engine::String::create(name);//.append("_magnitude");
	plotdata.data = magpoints;
	plotdata.phasedata = phasepoints;
	plotdata.color = color;

	RpGui::context->openedplots.pushBack(plotdata);
}


//removes a plot
void removePlot(char* name) {
	sizeptr index = 0;
	for (auto& plt : RpGui::context->openedplots) {
		if (Base::stringCompare(name, plt.name.getC_Str()) == true) {

			//release the memory of the plot data and name
			Engine::ArrayList<glm::vec2>::destroy(&plt.data);
			Engine::ArrayList<glm::vec2>::destroy(&plt.phasedata);
			Engine::String::destroy(&plt.name);

			RpGui::INFO << "Removing plot with name: " << name << "\n";
			RpGui::context->openedplots.remove(index);
			index++;
		}
	}
}



template<typename T>
void bind_array(py::module& m, const char* name) {
	py::class_<Engine::ArrayList<T>>(m, name)
		.def("__len__", &Engine::ArrayList<T>::getCount)

		.def("__getitem__", [](Engine::ArrayList<T>& a, size_t i) -> T& {

			if (i >= a.getCount()) {
				throw py::index_error();
			}
			return a[i];
		}, py::return_value_policy::reference_internal)

		.def("__setitem__", [](Engine::ArrayList<T>& a, size_t i, const T& v) {
			if (i >= a.getCount()) {
				throw py::index_error();
			}
			a[i] = v;
		});
}

// Bind it to a Python module
PYBIND11_EMBEDDED_MODULE(RpGui, m) {

	m.doc() = "C++ functions for my RpGui";

	bind_array<RpGui::Filter>(m, "FilterList");

	py::class_<glm::vec4>(m, "Vec4")
		.def(py::init<float, float, float, float>())
		.def_readwrite("x", &glm::vec4::x)
		.def_readwrite("y", &glm::vec4::y)
		.def_readwrite("z", &glm::vec4::z)
		.def_readwrite("w", &glm::vec4::w);

	py::enum_<RpGui::FilterType>(m, "FilterType")
		.value("LOWPASS", RpGui::FilterType::LOWPASS)
		.value("HIGHPASS", RpGui::FilterType::HIGHPASS)
		.value("BANDSTOP", RpGui::FilterType::BANDSTOP)
		.value("BANDPASS", RpGui::FilterType::BANDPASS)
		.value("ALLPASS", RpGui::FilterType::ALLPASS)
		.value("RESONANCE_ANTI_RESONANCE", RpGui::FilterType::RESONANCE_ANTI_RESONANCE)
		.value("COEFFICIENTS", RpGui::FilterType::COEFFICIENTS);

	py::class_<RpGui::Filter>(m, "Filter")
		.def_readwrite("type", &RpGui::Filter::type)
		.def_readwrite("cutoff", &RpGui::Filter::cutoff)
		.def_readwrite("Qfactor", &RpGui::Filter::Qfactor)
		.def_readwrite("antiQfactor", &RpGui::Filter::antiQfactor)
		.def_readwrite("df", &RpGui::Filter::df)
		.def("recalculate", [](RpGui::Filter& f) {
		recalculateFilter(&f);
			})

		.def("getCoeffs", [](RpGui::Filter& self) {
		return py::array_t<double>(
			6,
			(double*)&self.coeffs,
			py::cast(&self) // tie lifetime to object
		);
			});

	m.def("getTransferFunction", [](const char* name) -> RpGui::TransferFunction& {
		for (auto& tf : RpGui::context->activetransferfunctions) {
			if (Base::stringCompare(name, tf.name.getC_Str()) == true) {
				return tf;
			}
		}

		//if no transfer function with the given name exists, create a new one and return it
		RpGui::TransferFunction newTf;
		newTf.name = Engine::String::create(name);
		newTf.filters = Engine::ArrayList<RpGui::Filter>::create(0);
		return RpGui::context->activetransferfunctions.pushBack(newTf);

		}, py::return_value_policy::reference);

	py::class_<RpGui::TransferFunction>(m, "TransferFunction")
		.def("WriteToRp", [](RpGui::TransferFunction& tf, RpGui::Filter& filter) {
			sentFilterToRp(filter, RpGui::targetfs, &tf.connection);
		}, py::arg("Filter"))

		.def_readwrite("filters", &RpGui::TransferFunction::filters)

		.def("sentCommandToRp", [](RpGui::TransferFunction& tf, const char* command) {
			if (tf.connection.open) {
				tf.connection.commandqueue.push({ Engine::String::create(command) });
				ReleaseSemaphore(tf.connection.semaphore, 1, nullptr);
			}
		}, py::arg("command"));

		m.def("addPlot", &addPlot, "adds a plot to the GUI with the given frequency and magnitude data and name",
			py::arg("freq"), py::arg("magnitude"), py::arg("phase"), py::arg("name"), py::arg("color"));
		

		m.def("removePlot", &removePlot, "removes a plot from the GUI with the given name",
			py::arg("name"));

		m.def("setTitle", [](const char* title) {
			RpGui::context->plottitle.set(title);
			}, py::arg("title"));
}
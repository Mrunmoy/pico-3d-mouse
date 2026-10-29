p3d_add_module(p3d_domain_calibration_store
    ALIAS p3d::domain::calibration_store
    SOURCES CalibrationStore.cpp
    PUBLIC_DEPS p3d::hal
)

p3d_add_tests()

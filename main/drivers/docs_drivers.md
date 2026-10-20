\dir 

\brief Peripheral drivers

<hr /> 

Interactions with external devices is restricted to be done through dedicated driver implementations with separated headers into `include` and `src`. Device drivers should only interact with their device and remain single-purpose. If more complex operations are needed that functionality should be implemented as a utility (in `/main/utilities`). 



////////////////////////////////////////////////////////////////////////////////////
//                                                                                //
// Part of Tigris, under the MIT License.                                         //
// You may not use this file except in compliance with the License.               //
// See `https://github.com/12Thanjo/Tigris/blob/main/LICENSE`for info.            //
//                                                                                //
////////////////////////////////////////////////////////////////////////////////////



#pragma once

#include <mdspan>


namespace tigris{


	class Matrix{
		public:
			Matrix(size_t mat_width, size_t mat_height)
				: _width(mat_width), _height(mat_height), _data(mat_width * mat_height) {}

			Matrix(size_t mat_width, size_t mat_height, evo::ArrayProxy<float> mat_data)
				: _width(mat_width), _height(mat_height), _data(mat_data.begin(), mat_data.end()) {
				evo::debugAssert(
					this->width() * this->height() == this->_data.size(), "Dimensions and data do not match"
				);
			}

			Matrix(size_t mat_width, size_t mat_height, std::initializer_list<float> mat_data)
				: _width(mat_width), _height(mat_height), _data(mat_data.begin(), mat_data.end()) {
				evo::debugAssert(
					this->width() * this->height() == this->_data.size(), "Dimensions and data do not match"
				);
			}

			Matrix(size_t mat_width, size_t mat_height, std::vector<float>&& mat_data)
				: _width(mat_width), _height(mat_height), _data(std::move(mat_data)) {
				evo::debugAssert(
					this->width() * this->height() == this->_data.size(), "Dimensions and data do not match"
				);
			}


			EVO_NODISCARD static auto identity(size_t dimension) -> Matrix {
				auto output = Matrix(dimension, dimension);
				std::memset(output._data.data(), 0, output._data.size() * sizeof(float));
				for(size_t i = 0; i < dimension; i+=1){
					output[i, i] = 1.0f;
				}
				return output;
			}


			EVO_NODISCARD static auto random(size_t mat_width, size_t mat_height) -> Matrix {
				auto data = std::vector<float>();
				
				for(size_t i = 0; i < mat_width * mat_height; i+=1){
					data.emplace_back(float(evo::random01()));
				}

				return Matrix(mat_width, mat_height, std::move(data));
			}



			~Matrix() = default;


			EVO_NODISCARD auto operator==(const Matrix&) const -> bool = default;



			EVO_NODISCARD auto width() const -> size_t { return this->_width; }
			EVO_NODISCARD auto height() const -> size_t { return this->_height; }


			EVO_NODISCARD auto operator[](size_t x, size_t y) const -> const float& {
				return  std::mdspan(this->_data.data(), this->height(), this->width())[y, x];
			}
			EVO_NODISCARD auto operator[](size_t x, size_t y) -> float& {
				return  std::mdspan(this->_data.data(), this->height(), this->width())[y, x];
			}




			EVO_NODISCARD auto operator*(const Matrix& rhs) const -> Matrix {
				evo::debugAssert(this->width() == rhs.height(), "Invalid dimensions for multiplication");

				auto output = Matrix(rhs.width(), this->height());
				
				for(size_t x = 0; x < output.width(); x+=1){
					for(size_t y = 0; y < output.height(); y+=1){
						float value = 0.0f;

						for(size_t i = 0; i < this->width(); i+=1){

							const float a = this->operator[](i, y);
							const float b = rhs[x, i];

							value += a * b;
						}

						output[x, y] = value;
					}
				}

				return output;
			}

			auto operator*=(const Matrix& rhs) -> Matrix {
				evo::debugAssert(this->width() == rhs.height(), "Invalid dimensions for multiplication");

				*this = *this * rhs;
				return *this;
			}
			



			EVO_NODISCARD auto toString() const -> std::string {
				auto output = std::string();

				for(size_t y = 0; y < this->height(); y+=1){
					for(size_t x = 0; x < this->width(); x+=1){
						output += std::to_string(this->operator[](x, y));

						if(x + 1 < this->width()){ output += ' '; }
					}

					if(y + 1 < this->height()){ output += '\n'; }
				}

				return output;
			}


			EVO_NODISCARD auto data() -> std::span<float> {
				return this->_data;
			}


	
		private:
			size_t _width;
			size_t _height;
			std::vector<float> _data;
	};

	
}



template<>
struct std::formatter<tigris::Matrix>{
    constexpr auto parse(std::format_parse_context& ctx) const -> auto {
        return ctx.begin();
    }

    auto format(const tigris::Matrix& matrix, std::format_context& ctx) const -> auto {
        return std::format_to(ctx.out(), "{}", matrix.toString());
    }
};
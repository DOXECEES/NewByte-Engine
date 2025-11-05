#ifndef NBUI_SRC_DYNAMICFLATMATRIX_HPP
#define NBUI_SRC_DYNAMICFLATMATRIX_HPP

#include <NbCore.hpp>

#include <vector>

namespace nbstl
{
	template<typename T>
	class DynamicFlatMatrix
	{
	public:
		DynamicFlatMatrix(const size_t rows, const size_t cols) noexcept
			: columns(cols)
			, rows(rows)
		{
			data.resize(cols * rows);
			
		}

		void insert(const T& item, const size_t row, const size_t column) noexcept
		{
			NB_ASSERT(row < rows, "Row larger than rows");
			NB_ASSERT(column < columns, "Column larger than columns");

			data[row * columns + column] = item;
		}

		NB_NODISCARD T get(const size_t row, size_t column) const noexcept
		{
			NB_ASSERT(row < rows, "Row larger than rows");
			NB_ASSERT(column < columns, "Column larger than columns");

			return data.at(row * columns + column);
		}

	private:
		size_t columns;
		size_t rows;
		std::vector<T> data;
	};

};

#endif

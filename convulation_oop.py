import tkinter as tk
from tkinter import ttk
from tkinter import simpledialog
from random import randint
from collections import deque


def rgb(params):
    red, green, blue = params[0], params[1], params[2]
    hexas = [red, green, blue]
    hexa_str = ""
    for hexa in hexas:
        hexa = str(hex(hexa)).removeprefix("0x").upper()
        if hexa.__len__() == 1:
            hexa = "0" + hexa
        hexa_str += hexa
    return "#" + hexa_str


class ColorFrame(ttk.Frame):
    def __init__(self, container, canvasObj, memoObj):
        self.red = tk.IntVar()
        self.green = tk.IntVar()
        self.blue = tk.IntVar()
        self.redStr = tk.StringVar(value="Red: " + str(self.red.get()))
        self.greenStr = tk.StringVar(value="Green: " + str(self.green.get()))
        self.blueStr = tk.StringVar(value="Blue: " + str(self.blue.get()))
        self.colorStr = tk.StringVar(
            value=rgb((self.red.get(), self.green.get(), self.blue.get())))
        self.canvasObj = canvasObj
        self.memoObj = memoObj
        super().__init__(container)
        self.canvasObj.canvas.bind(
            "<B1-Motion>",
            lambda event: self.canvasObj.highlight(
                event, self.red.get(), self.green.get(), self.blue.get())
        )
        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=1)
        self.grid_columnconfigure(2, weight=1)
        self.grid_columnconfigure(3, weight=1)
        self.grid_rowconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)
        self.__createWidgets()

    def __createWidgets(self):
        redLabel = ttk.Label(
            self,
            textvariable=self.redStr,
            font=("Helvetica", 14),
        )
        greenLabel = ttk.Label(
            self,
            textvariable=self.greenStr,
            font=("Helvetica", 14),
        )
        blueLabel = ttk.Label(
            self,
            textvariable=self.blueStr,
            font=("Helvetica", 14),
        )
        redEntry = ttk.Scale(
            self,
            from_=0,
            to=255,
            orient="horizontal",
            variable=self.red,
            command=lambda event: self.__scale(
                (self.red.get(), self.green.get(), self.blue.get()),
                (self.redStr, self.greenStr, self.blueStr),
                color
            ),
        )
        greenEntry = ttk.Scale(
            self,
            from_=0,
            to=255,
            orient="horizontal",
            variable=self.green,
            command=lambda event: self.__scale(
                (self.red.get(), self.green.get(), self.blue.get()),
                (self.redStr, self.greenStr, self.blueStr),
                color
            ),
        )
        blueEntry = ttk.Scale(
            self,
            from_=0,
            to=255,
            orient="horizontal",
            variable=self.blue,
            command=lambda event: self.__scale(
                (self.red.get(), self.green.get(), self.blue.get()),
                (self.redStr, self.greenStr, self.blueStr),
                color
            ),
        )
        color = tk.Canvas(self, width=40, height=40, bg=self.colorStr.get())
        fillBtn = ttk.Button(
            self,
            text="Fill",
            command=lambda: self.__fillOne(),
        )
        redLabel.grid(column=0, row=0, padx=5, pady=5)
        redEntry.grid(column=0, row=1, padx=5, pady=5)
        greenLabel.grid(column=1, row=0, padx=5, pady=5)
        greenEntry.grid(column=1, row=1, padx=5, pady=5)
        blueLabel.grid(column=2, row=0, padx=5, pady=5)
        blueEntry.grid(column=2, row=1, padx=5, pady=5)
        color.grid(column=3, row=0, rowspan=2, padx=5, pady=5)
        fillBtn.grid(column=3, row=1, padx=5, pady=5)

    def __scale(self, cTuple, cStrTuple, cCanvas):
        cStrTuple[0].set(f"Red: %d" % cTuple[0])
        cStrTuple[1].set(f"Green: %d" % cTuple[1])
        cStrTuple[2].set(f"Blue: %d" % cTuple[2])
        self.colorStr.set(rgb(cTuple))
        cCanvas.configure(bg=self.colorStr.get())

    def __fillOne(self):
        for i in range(self.canvasObj.ROW):
            for j in range(self.canvasObj.COL):
                self.canvasObj.pixels[i][j] = (
                    self.red.get(), self.green.get(), self.blue.get())
        self.memoObj.addToMemo()
        self.canvasObj.fillImg(self.canvasObj.pixels)


class CanvasFrame(ttk.Frame):
    def __init__(self, container, row, col, pixel):
        self.container = container
        self.ROW, self.COL = row, col
        self.PIXEL = pixel
        self.canvas = tk.Canvas(self.container)
        self.pixels = self.__initSeed(self.ROW, self.COL)
        super().__init__()
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(0, weight=1)
        self.__createWidgets()

    def __initSeed(self, row, col):
        seed = []
        for i in range(row):
            subSeed = []
            for j in range(col):
                subSeed.append((255, 255, 255))
            seed.append(subSeed)
        return seed

    def __createWidgets(self):
        self.canvas.master = self
        self.canvas.configure(width=self.COL * self.PIXEL)
        self.canvas.configure(height=self.ROW * self.PIXEL)
        self.canvas.configure(bg="#FFFFFF")
        self.canvas.grid(column=0, row=0, columnspan=2)

    def __boundCheck(self, row, col):
        return (col >= 0
                and row >= 0
                and col < self.COL
                and row < self.ROW
                )

    def fillImg(self, colors):
        self.canvas.delete(tk.ALL)
        for i in range(self.ROW):
            for j in range(self.COL):
                self.canvas.create_rectangle(
                    (self.PIXEL * j, self.PIXEL * i),
                    (self.PIXEL * (j + 1), self.PIXEL * (i + 1)),
                    fill=rgb(colors[i][j]),
                    outline="",
                )

    def highlight(self, event, r, g, b):
        xIndex = (event.x) // self.PIXEL
        yIndex = (event.y) // self.PIXEL
        if (self.__boundCheck(yIndex, xIndex)):
            self.pixels[yIndex][xIndex] = (
                (5 * self.pixels[yIndex][xIndex][0] + r) // 6,
                (5 * self.pixels[yIndex][xIndex][1] + g) // 6,
                (5 * self.pixels[yIndex][xIndex][2] + b) // 6,
            )
            self.canvas.create_rectangle(
                (self.PIXEL * xIndex, self.PIXEL * yIndex),
                (self.PIXEL * (xIndex + 1), self.PIXEL * (yIndex + 1)),
                fill=rgb(self.pixels[yIndex][xIndex]),
                outline="",
            )

    def draw(self, event, r, g, b):
        xIndex = (event.x) // self.PIXEL
        yIndex = (event.y) // self.PIXEL
        if (self.__boundCheck(yIndex, xIndex)):
            self.pixels[yIndex][xIndex] = (r, g, b)
            self.canvas.create_rectangle(
                (self.PIXEL * xIndex, self.PIXEL * yIndex),
                (self.PIXEL * (xIndex + 1), self.PIXEL * (yIndex + 1)),
                fill=rgb(self.pixels[yIndex][xIndex]),
                outline="",
            )


class FunctionFrame(ttk.Frame):
    def __init__(self, container, canvasObj, memoObj, colorObj):
        self.canvasObj = canvasObj
        self.memoObj = memoObj
        self.colorObj = colorObj
        super().__init__(container)
        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=1)
        self.grid_columnconfigure(2, weight=1)
        self.grid_columnconfigure(3, weight=1)
        self.grid_columnconfigure(4, weight=1)
        self.grid_rowconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)
        self.__createWidgets()

    def __createWidgets(self):
        refreshBtn = ttk.Button(
            self, text="Refresh", command=lambda: self.__randImg()
        )
        blurBtn = ttk.Button(
            self, text="Blur", command=lambda: self.__blurImg(self.canvasObj.pixels))
        drawBtn = ttk.Button(
            self, text="Draw", command=lambda: self.__setDraw())
        highlightBtn = ttk.Button(
            self, text="Highlight", command=lambda: self.__setHighlight())
        invertBtn = ttk.Button(
            self, text="Invert", command=lambda: self.__invert()
        )
        mirrorSideBtn = ttk.Button(
            self, text="Mirror Side", command=lambda: self.__mirrorSide()
        )
        mirrorUpBtn = ttk.Button(
            self, text="Mirror Up", command=lambda: self.__mirrorUp()
        )
        if self.canvasObj.ROW == self.canvasObj.COL:
            rotateRightBtn = ttk.Button(
                self, text="Rotate Right", command=lambda: self.__rotateRight()
            )
            rotateLeftBtn = ttk.Button(
                self, text="Rotate Left", command=lambda: self.__rotateLeft()
            )

        drawBtn.grid(column=0, row=0, padx=5, pady=5)
        highlightBtn.grid(column=0, row=1, padx=5, pady=5)
        refreshBtn.grid(column=1, row=0, padx=5, pady=5)
        blurBtn.grid(column=1, row=1, padx=5, pady=5)
        mirrorUpBtn.grid(column=2, row=0, padx=5, pady=5)
        mirrorSideBtn.grid(column=2, row=1, padx=5, pady=5)
        if self.canvasObj.ROW == self.canvasObj.COL:
            rotateRightBtn.grid(column=3, row=0, padx=5, pady=5)
            rotateLeftBtn.grid(column=3, row=1, padx=5, pady=5)
        invertBtn.grid(column=4, row=0, padx=5, pady=5)

    def __randImg(self):
        for i in range(self.canvasObj.ROW):
            for j in range(self.canvasObj.COL):
                self.canvasObj.pixels[i][j] = (
                    randint(0, 255), randint(0, 255), randint(0, 255))
        self.memoObj.addToMemo()
        self.canvasObj.fillImg(self.canvasObj.pixels)

    def __convulationAt(self, seed, matrix, i, j):
        value = []
        entry = 0
        matrixRows = matrix.__len__()
        matrixCols = matrix[0].__len__()
        matrixRowAvg = matrix.__len__() // 2
        matrixColAvg = matrix[0].__len__() // 2
        visibleRows = min(
            i + matrixRowAvg + 1, seed.__len__() - i + matrixRowAvg, matrixRows
        )
        visibleCols = min(
            j + matrixColAvg + 1, seed[0].__len__() -
            j + matrixColAvg, matrixCols
        )
        visibleGrid = visibleRows * visibleCols
        for vpos in range(seed[0][0].__len__()):
            for row in range(matrixRows):
                for col in range(matrixCols):
                    if (
                        row + i - matrixRowAvg >= 0
                        and col + j - matrixColAvg >= 0
                        and row + i - matrixRowAvg < seed.__len__()
                        and col + j - matrixColAvg < seed[0].__len__()
                    ):
                        entry += (
                            seed[row + i - matrixRowAvg][col +
                                                         j - matrixColAvg][vpos]
                            * matrix[row][col]
                            * (matrixRows * matrixCols / visibleGrid)
                        )
            value.append(int(entry))
            entry = 0
        return tuple(value)

    def __blur(self, seed):
        blurredSeed = []
        matrix = [[1 / 9 for x in range(3)] for y in range(3)]
        """
        matrix = [
            [0.003, 0.013, 0.022, 0.013, 0.003],
            [0.013, 0.060, 0.098, 0.060, 0.013],
            [0.022, 0.098, 0.162, 0.098, 0.013],
            [0.013, 0.060, 0.098, 0.060, 0.013],
            [0.003, 0.013, 0.022, 0.013, 0.003],
        ]
        """
        for i in range(seed.__len__()):
            blurredRow = []
            for j in range(seed[0].__len__()):
                pixel = self.__convulationAt(seed, matrix, i, j)
                blurredRow.append(pixel)
            blurredSeed.append(blurredRow)
        return blurredSeed

    def __blurImg(self, seed):
        b = self.__blur(seed)
        for i in range(b.__len__()):
            for j in range(b[0].__len__()):
                seed[i][j] = b[i][j]
        self.memoObj.addToMemo()
        self.canvasObj.fillImg(seed)

    def __setDraw(self):
        self.canvasObj.canvas.unbind("<B1-Motion>")
        self.canvasObj.canvas.bind(
            "<B1-Motion>",
            lambda event: self.canvasObj.draw(
                event, self.colorObj.red.get(), self.colorObj.green.get(), self.colorObj.blue.get()),
        )

    def __setHighlight(self):
        self.canvasObj.canvas.unbind("<B1-Motion>")
        self.canvasObj.canvas.bind(
            "<B1-Motion>",
            lambda event: self.canvasObj.highlight(
                event, self.colorObj.red.get(), self.colorObj.green.get(), self.colorObj.blue.get()),
        )

    def __invert(self):
        for i in range(self.canvasObj.ROW):
            for j in range(self.canvasObj.COL):
                color = self.canvasObj.pixels[i][j]
                self.canvasObj.pixels[i][j] = (
                    255 - color[0], 255 - color[1], 255 - color[2])
        self.memoObj.addToMemo()
        self.canvasObj.fillImg(self.canvasObj.pixels)

    def __mirrorSide(self):
        copy = self.memoObj.copyMatrix(self.canvasObj.pixels)
        for i in range(self.canvasObj.ROW):
            for j in range(self.canvasObj.COL):
                self.canvasObj.pixels[i][j] = copy[i][self.canvasObj.COL - j - 1]
        self.memoObj.addToMemo()
        self.canvasObj.fillImg(self.canvasObj.pixels)

    def __mirrorUp(self):
        copy = self.memoObj.copyMatrix(self.canvasObj.pixels)
        for i in range(self.canvasObj.ROW):
            for j in range(self.canvasObj.COL):
                self.canvasObj.pixels[i][j] = copy[self.canvasObj.ROW - i - 1][j]
        self.memoObj.addToMemo()
        self.canvasObj.fillImg(self.canvasObj.pixels)

    def __rotateRight(self):
        copy = self.memoObj.copyMatrix(self.canvasObj.pixels)
        for i in range(self.canvasObj.ROW):
            for j in range(self.canvasObj.COL):
                self.canvasObj.pixels[i][j] = copy[self.canvasObj.COL - j - 1][i]
        self.memoObj.addToMemo()
        self.canvasObj.fillImg(self.canvasObj.pixels)

    def __rotateLeft(self):
        copy = self.memoObj.copyMatrix(self.canvasObj.pixels)
        for i in range(self.canvasObj.ROW):
            for j in range(self.canvasObj.COL):
                self.canvasObj.pixels[i][j] = copy[j][self.canvasObj.ROW - i - 1]
        self.memoObj.addToMemo()
        self.canvasObj.fillImg(self.canvasObj.pixels)


class Memo():
    def __init__(self, canvasObj):
        self.canvasObj = canvasObj
        self.memoZ = deque()
        self.memoY = deque()

    def __enqueuePixels(self, queue):
        mem = []
        for i in range(self.canvasObj.ROW):
            memRow = []
            for j in range(self.canvasObj.COL):
                memRow.append(self.canvasObj.pixels[i][j])
            mem.append(memRow)
        queue.append(mem)

    def copyMatrix(self, m):
        mem = []
        for i in range(m.__len__()):
            memRow = []
            for j in range(m[0].__len__()):
                memRow.append(m[i][j])
            mem.append(memRow)
        return mem

    def addToMemo(self):
        self.__enqueuePixels(self.memoZ)
        while self.memoY.__len__() > 0:
            self.memoY.pop()
        if self.memoZ.__len__() > 20:
            self.memoZ.popleft()

    def ctrlZ(self):
        if self.memoZ.__len__() > 1:
            z = self.memoZ.pop()
            self.canvasObj.pixels = self.copyMatrix(z)
            self.__enqueuePixels(self.memoY)
            z = self.memoZ.pop()
            self.canvasObj.pixels = self.copyMatrix(z)
            self.memoZ.append(z)
            self.canvasObj.fillImg(self.canvasObj.pixels)

    def ctrlY(self):
        if self.memoY.__len__() > 0:
            y = self.memoY.pop()
            self.canvasObj.pixels = self.copyMatrix(y)
            self.__enqueuePixels(self.memoZ)
            self.canvasObj.fillImg(self.canvasObj.pixels)


class Menubar():
    def __init__(self, root, canvasObj):
        self.root = root
        self.canvasObj = canvasObj
        self.menubar = tk.Menu(root)
        root.configure(menu=self.menubar)
        self.__createMenu()

    def __createMenu(self):
        fileMenu = tk.Menu(self.menubar, tearoff=False)
        fileMenu.add_command(
            label="New File",
            command=lambda: self.__openNew()
        )
        fileMenu.add_command(
            label="Save"
        )
        fileMenu.add_command(
            label="Open..."
        )
        self.menubar.add_cascade(
            label="File",
            menu=fileMenu
        )
        editMenu = tk.Menu(self.menubar, tearoff=False)
        editMenu.add_command(
            label="Resize",
            command=lambda: self.__resizeCanvas()
        )
        self.menubar.add_cascade(
            label="Edit",
            menu=editMenu
        )

    def __openNew(self):
        ResizeDialog(self.root, self.canvasObj)

    def __resizeCanvas(self):
        SizeDialog(self.root, self.canvasObj)


class SizeDialog(tk.Toplevel):
    def __init__(self, parent, canvasObj):
        self.canvasObj = canvasObj
        self.parent = parent
        super().__init__(parent)
        self.pixel = tk.StringVar()
        self.title("Set Size")
        self.resizable(False, False)
        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)
        self.__createWidgets()

    def __createWidgets(self):
        pixelLabel = ttk.Label(
            self, text="Enter new pixel size: ", font=("Helvetica", 14))
        pixelSpin = ttk.Spinbox(self, from_=0, to=200,
                                increment=1, textvariable=self.pixel)
        okBtn = ttk.Button(self, text="OK", command=lambda: self.__okPress())
        cancelBtn = ttk.Button(self, text="Cancel",
                               command=lambda: self.__cancelPress())
        pixelLabel.grid(row=0, column=0, padx=5, pady=5)
        pixelSpin.grid(row=0, column=1, padx=5, pady=5)
        okBtn.grid(row=1, column=0, padx=5, pady=5)
        cancelBtn.grid(row=1, column=1, padx=5, pady=5)

    def __okPress(self):
        self.parent.restruct(
            self.canvasObj.ROW, self.canvasObj.COL, int(self.pixel.get()))
        self.destroy()

    def __cancelPress(self):
        self.destroy()


class ResizeDialog(tk.Toplevel):
    def __init__(self, parent, canvasObj):
        self.canvasObj = canvasObj
        self.parent = parent
        super().__init__(parent)
        self.row = tk.StringVar()
        self.col = tk.StringVar()
        self.title("New File")
        self.resizable(False, False)
        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=1)
        self.grid_rowconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)
        self.grid_rowconfigure(2, weight=1)
        self.__createWidgets()

    def __createWidgets(self):
        rowLabel = ttk.Label(
            self, text="Enter height in pixels: ", font=("Helvetica", 14))
        rowSpin = ttk.Spinbox(self, from_=0, to=2000,
                              increment=1, textvariable=self.row)
        colLabel = ttk.Label(
            self, text="Enter width in pixels: ", font=("Helvetica", 14))
        colSpin = ttk.Spinbox(self, from_=0, to=2000,
                              increment=1, textvariable=self.col)
        okBtn = ttk.Button(self, text="OK", command=lambda: self.__okPress())
        cancelBtn = ttk.Button(self, text="Cancel",
                               command=lambda: self.__cancelPress())
        rowLabel.grid(row=0, column=0, padx=5, pady=5)
        colLabel.grid(row=1, column=0, padx=5, pady=5)
        rowSpin.grid(row=0, column=1, padx=5, pady=5)
        colSpin.grid(row=1, column=1, padx=5, pady=5)
        okBtn.grid(row=2, column=0, padx=5, pady=5)
        cancelBtn.grid(row=2, column=1, padx=5, pady=5)

    def __okPress(self):
        self.parent.restruct(int(self.row.get()), int(
            self.col.get()), self.canvasObj.PIXEL)
        self.destroy()

    def __cancelPress(self):
        self.destroy()


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.container = ttk.Frame(self)
        self.title("Convulation")
        self.container.grid_columnconfigure(0, weight=1)
        self.container.grid_rowconfigure(0, weight=1)
        self.container.grid_rowconfigure(1, weight=1)
        self.container.grid_rowconfigure(2, weight=1)
        self.__createWidgets(16, 16, 20)
        self.container.grid(row=1, column=0)

    def __createWidgets(self, row, col, pixel):
        canvas = CanvasFrame(self.container, row, col, pixel)
        memo = Memo(canvas)
        colors = ColorFrame(self.container, canvas, memo)
        funcs = FunctionFrame(self.container, canvas, memo, colors)
        Menubar(self, canvas)
        canvas.canvas.bind(
            "<B1-ButtonRelease>",
            lambda event: memo.addToMemo(),
        )
        self.bind(
            "<Control-z>",
            lambda event: memo.ctrlZ(),
        )
        self.bind(
            "<Control-y>",
            lambda event: memo.ctrlY(),
        )
        memo.addToMemo()
        colors.grid(row=1, column=0)
        funcs.grid(row=2, column=0)

    def restruct(self, row, col, pixel):
        self.container.destroy()
        self.container = ttk.Frame(self)
        self.container.grid_columnconfigure(0, weight=1)
        self.container.grid_rowconfigure(0, weight=1)
        self.container.grid_rowconfigure(1, weight=1)
        self.container.grid_rowconfigure(2, weight=1)
        self.__createWidgets(row, col, pixel)
        self.container.grid(row=1, column=0)


if __name__ == "__main__":
    app = App()
    try:
        from ctypes import windll

        windll.shcore.SetProcessDpiAwareness(1)
    finally:
        app.mainloop()
